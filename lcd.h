/*
 * @file lcd.h
 * @brief LCD functions header file
 */

#include "stdint.h"
#ifndef LCD_H_
#define LCD_H_

/**
 * Resets the LCD
 */
extern void lcd_reset();

/**
 * Sending the command cmd to LCD display
 */
extern void lcd_command(char cmd);

/**
 * Sets the display to start mode with 4 zeros
 */
extern void lcd_begin();

/**
 * LCD display initialization
 * */
extern void lcd_initialization();

/**
 * Printing out the data
 * */
extern void lcd_display(unsigned char dat);

/**
 * Displays specific text starting from pos
 */
extern void lcd_text(char *text, int pos);


#endif
