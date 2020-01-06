/**
 * @file main.c
 * @brief Electric lock implementation
 *
 * The programs is intended for simulating an electric lock functionality.
 * In order to unlock the lock we need to input the right 4-digit password.
 * The password is predefined in the program, and along with that
 * it cannot be changed. The program starts with the first digit selected
 * on the LCD. Using potentiometer we can change the value of the first
 * digit, and confirm our selection using the first button on the left.
 * The process is similar for remaining three digits. Finally we submit
 * our selection for the fourth digit we get a feedback which indicates
 * if we got the password right. First LED indicates success while the
 * other indicates failure. Both LEDs are on for three seconds.
 *
 * @date Dec 2019
 * @author Senad Kurtisi
 */

#include <msp430.h> 
#include <stdint.h>
#include <stdlib.h>
#include "lcd.h"

/** LED port output */
#define LED_PORT        P4OUT
/** LED port direction */
#define LED_PORT_DIR    P4DIR

/** Button port direction */
#define BUTTON_PORT_DIR P2DIR
/** Button interrupt enable */
#define BUTTON_INT_EN   P2IE
/** Button flag */
#define BUTTON_FLAG     P2IFG
/** Button edge select */
#define BUTTON_EDGE_SEL P2IES

/** One second calculated with the ACLK */
#define ONE_SECOND (32768-1)
/** ASCII representation for zero */
#define ASCII_CONV      0x30
/** Counter of seconds */
volatile uint8_t seconds=0;
/** Lock password */
volatile uint8_t password[4] = {1,2,3,4};
/** Password guess */
volatile uint8_t guess[4] = {0,0,0,0};
/** Password guess counter */
volatile uint8_t data_cnt = 0;
/** Button press indicator */
volatile int8_t pressed = 0;
/** Correct guess indicator */
volatile uint8_t correct = 1;
/** Conversion allowed condition*/
volatile uint8_t conversion_allowed = 1;

/**
 * @brief Turning on LEDs
 *
 * Turns on the adequate LED with respect
 * to the validity of the password guess.
 *
 */
void turn_on_led()
{

    TA0CTL = TASSEL__ACLK | MC__UP; // selecting the up mode
    switch(correct){
    case 0:
        lcd_command(0x01);          // clears the screen
        lcd_text("WRONG",0x85);     // writes the text for wrong pw input
        __delay_cycles(1000);
        lcd_text("PASSWORD",0xC4);  // writes the text for wrong pw input

        TA0CCR0 = ONE_SECOND;       // set the CCR0 for measuring 1s
        TA0CCTL0 |= CCIE;           // enable interrupt for CCR0
        LED_PORT |= BIT4;           // turns on wrong answer LED
        break;
    case 1:
        lcd_command(0x01);          // clears the screen
        lcd_text("UNLOCKED",0x84);  // writes the text for correct pw input

        TA0CCR0 = ONE_SECOND;       // set the CCR0 for measuring 1s
        TA0CCTL0 |= CCIE;           // enable interrupt for CCR0
        LED_PORT |= BIT3;           // turns on right answer LED
        break;
    }
    return;
}

/**
 * @brief Password check
 *
 * Checks password guess validity by
 * comparing the master password and
 * the password guess.
 *
 */
void check_password()
{
    ADC12CTL0 &= ~ADC12ENC;         // disable conversion
    ADC12IE &= ~ADC12IE0;           // disable interrupt for MEM0
    TA0CCR0 = 0;                    // sets the compare value to 0
    TA0CTL = MC__STOP;              // stops the timer

    int i;                          // current digit iterator used for password validation
    data_cnt = 0;
    for(i=0;i<4;i++){
        if(guess[i]!=password[i]){
            correct = 0;            // sets the indicator that our guess is wrong
            break;                  // exits the password check loop
        }
    }
    turn_on_led();                  // calls the function for turning on adequate LED
    return;
}

/**
 * @brief Main program
 */
int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Configure LED port
    LED_PORT &= ~BIT3;          // reset P4.3
    LED_PORT &= ~BIT4;          // reset P4.4
    LED_PORT &= ~BIT5;          // reset P4.5

    LED_PORT_DIR |= BIT3;       // set P4.3 as out
    LED_PORT_DIR |= BIT4;       // set P4.4 as out
    LED_PORT_DIR |= BIT5;       // set P4.5 as out

    // Configure button port
    BUTTON_PORT_DIR &= ~BIT4;   // set P2.4 as in
    BUTTON_EDGE_SEL |= BIT4;    // button interrupt on falling edge

    BUTTON_FLAG &= ~BIT4;       // clear IFG
    BUTTON_INT_EN |= BIT4;      // enable P2.4 interrupt

    // initialize ADC
    P7SEL |= BIT6;              // set P7.6 for ADC
    ADC12CTL0 = ADC12ON;        // turn on ADC
    ADC12CTL1 = ADC12SHS_1 | ADC12CONSEQ_2; // set SHS = 1 (TA0.0 used for SAMPCON) and repeat-single-channel mode
    ADC12MCTL0 = ADC12INCH_14;              // select channel 14
    ADC12CTL0 |= ADC12ENC;                  // enable conversion
    ADC12IE |= ADC12IE0;                    // enable interrupt for MEM0
    ADC12IFG &= ~ADC12IFG0;                 // clear interrupt flag for first channel of ADC

    // initialize timer
    TA0CCTL0 = OUTMOD_4;                    // use toggle outmode
    TA0CCTL0 &= ~CCIE;                      // disable interrupt for CCR0
    TA0CCR0 = 8191;                         // f_OUT = 2 Hz, f_ACLK = 32768 Hz => T_OUT = 16384.
                                            // TOGGLE outmod => T_OUT = 2 * T_CCR0 => T_CCR0 = 8192 (TA0CCR0 = T_CCR0 - 1)
    TA0CTL = TASSEL__ACLK | MC__UP;         // ACLK source, UP mode

    lcd_initialization();       // initialize and configure the LCD module
    __enable_interrupt();       // enable all interrupts

    while(1){
        if(pressed==1){
            if(conversion_allowed){
                guess[data_cnt] = (ADC12MEM0 >> 8) & 0x0F;    // adds digit guess to the vector
                data_cnt++;             // updates the digit counter
            }

            if(data_cnt > 3){
                conversion_allowed = 0;         // disables the conversion (software)
                check_password();               // checks the validity of the password guess
            }
            pressed = 0;                // resets the 'pressed' bit
        }
    }
}

/**
 * @brief Updating the current digit using A/D conversion
 *
 * Reads the change using ADC interrupt vector
 * and along with that updates current digit
 * on the LCD. Only the high nibble is used
 * per module 10.
 */
#pragma vector = ADC12_VECTOR
__interrupt void ADC12ISR (void)
{
    switch(ADC12IV){
    case 6:
        if(conversion_allowed){
            lcd_command(0x80+data_cnt);         // position the cursor for the current digit
            lcd_display(((ADC12MEM0 >> 8) & 0x0F) + ASCII_CONV);    // send high nibble of data (converted to ASCII)
        }
        break;
    default:
        break;
    }
    LED_PORT ^= BIT5;                       // LED for indicating that ADC is working (blinking)
    ADC12IFG &= ~ADC12IFG0;                 // clear interrupt flag for first channel of ADC
}

/**
 * @brief One second elapsed ISR
 *
 * Counts the number of seconds that have passed since
 * the adequate LED has been turned on. If 3 seconds
 * have passed, LED is turned off and the number of
 * seconds is reset. ISR is unavailable until next
 * password guess validation.
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A_CCR0_ISR(void)
{
    seconds++;                      // increments the number of seconds

    if(seconds>2){
        if(correct){
            LED_PORT &= ~BIT3;      // turns off the correct answer LED
        } else{
            LED_PORT &= ~BIT4;      // turns off the wrong answer LED
        }
        TA0CCTL0 &= ~CCIE;          // disable interrupt for CCR0
        lcd_command(0x01);          // clears the screen
        lcd_begin();                // sets the display back to initial state
        correct = 1;                // resets the correct password flag
        seconds = 0;                // resets the number of counted seconds
        TA0CCR0 = 8191;
        ADC12CTL0 |= ADC12ENC;      // re-enable the conversion
        conversion_allowed = 1;     // re-enable the conversion (software)
        ADC12IE |= ADC12IE0;        // enable interrupt for MEM0
    }
    TA0CCTL0 &= ~CCIFG;             // clears the CCR0 interrupt flag
}
