/*
 * @file lcd.c
 * @brief LCD functions file
 *
 * The file contains implementations of functions that
 * enable the LCD to show desired content. Commands and
 * relevant LCD hardware are defined as macros.
 */

#include <msp430.h>
#include <lcd.h>

/** LCD port output */
#define LCD_PORT        P8OUT
/** LCD port direction */
#define LCD_PORT_DIR    P8DIR

/** LCD enable pin */
#define LCD_EN            BIT3
/** LCD register select pin */
#define LCD_RS            BIT2
/** Mask upper four bits */
#define MASK_UPPER        0xF0
/** Mask all eight bits */
#define MASK_FULL         0xFF
/** 4 bit mode 5x7 font LCD */
#define MODE_FONT         0x28
/** LCD cursor no blink */
#define CURS_BLINK        0x0C
/** Automatic increment no display shift LCD mode command */
#define SHIFTR            0x06
/** Forcing cursor at the beginning of the first line command*/
#define DDRAM             0x80
/** Clear LCD screen */
#define CLEAR_SCREEN      0x01
/** ASCII code for zero */
#define ASCII_CONV        0x30

/**
 * @brief LCD reset
 *
 * Resets the LCD with respect to execution
 * time. After executing this function LCD
 * is ready for further use.
 */
extern void lcd_reset(){
    LCD_PORT_DIR = MASK_FULL;    // output mode
    LCD_PORT = MASK_FULL;
    __delay_cycles(20000);
    LCD_PORT = 0x30+LCD_EN;
    LCD_PORT = 0x30;
    __delay_cycles(10000);
    LCD_PORT = 0x30+LCD_EN;
    LCD_PORT = 0x30;
    __delay_cycles(1000);
    LCD_PORT = 0x30+LCD_EN;
    LCD_PORT = 0x30;
    __delay_cycles(1000);
    LCD_PORT = 0x20+LCD_EN;
    LCD_PORT = 0x20;
    __delay_cycles(1000);
}

/**
 * @brief LCD command
 *
 * Sends the command to LCD port in the
 * 4-bit mode with sending the upper nibble
 * of the command first.
 *
 * @param cmd The hex command to be sent
 */
extern void lcd_command (char cmd){
    // Send upper nibble
    LCD_PORT = (cmd & MASK_UPPER) | LCD_EN;
    LCD_PORT = (cmd & MASK_UPPER);

    // Send lower nibble
    LCD_PORT = ((cmd << 4) & MASK_UPPER) | LCD_EN;
    LCD_PORT = ((cmd << 4) & MASK_UPPER);
    __delay_cycles(4000);
}

/**
 * @brief Restart the beginning phase
 *
 * Sets the display in the "beginning"
 * state where all digits are 0 and cursor
 * is set at the first digit position.
 */
extern void lcd_begin(){
    int i;                          // current digit iterator used for zeros intialization
    lcd_command(DDRAM);

    for(i=0;i<4;i++){
        lcd_display(ASCII_CONV);    // sets the current digit to be zero
    }

    lcd_text("Enter PW",0xC1);      // displays text on the second line
    lcd_command(DDRAM);             // return the cursor to the first digit
}

/**
 * @brief LCD initialization
 *
 * Initializes the LCD by setting the
 * display mode, cursor and setting the
 * initial state of the LCD.
 */
extern void lcd_initialization(){
    lcd_reset();                    // initial reset of the LCD
    lcd_command(MODE_FONT);         // 4-bit mode - 2 line - 5x7 font.
    lcd_command(CURS_BLINK);        // display on with cursor blinking
    lcd_command(SHIFTR);            // no display shift, auto increment
    lcd_command(CLEAR_SCREEN);      // clears the screen
    lcd_begin();
}

/**
 * @brief Displays data
 *
 * Displays data on the LCD screen using
 * the 4-bit mode. Digits are limited to
 * number 9. In other words, password input
 * is processed per module 10. Upper nibble
 * of a character is sent first.
 *
 * @param dat Character to be displayed
 */
extern void lcd_display (unsigned char dat){
    if(dat>0x39 && dat<0x41){
        dat = 0x39;           // Limiting the current digit  not to be greater than 9
    }

    // Sending the high nibble of data
    LCD_PORT = ((dat & MASK_UPPER)|LCD_EN|LCD_RS);
    LCD_PORT = ((dat & MASK_UPPER)|LCD_RS);

    // Sending the low nibble of data
    LCD_PORT = (((dat << 4) & MASK_UPPER)|LCD_EN|LCD_RS);
    LCD_PORT = (((dat << 4) & MASK_UPPER)|LCD_RS);
    __delay_cycles(4000);
}

/**
 * @brief Text display
 *
 * Displays specific text on the screen
 * starting from the specified position.
 *
 * @param text Text to be displayed
 * @param pos Starting position for the text
 */
extern void lcd_text(char *text, int pos){
    lcd_command(pos);           // positions the cursor at "pos"

    while(*text){
        lcd_display(*text++);   // writes current character
    }
}
