// file: lcd_driver.h
// created by: Grant Wilk
// date created: 12/17/2019
// last modified: 1/5/2020
// description: Header file for lcd_driver.c

// Initializes the LCD pins and readys the LCD peripheral for use
void lcd_init(void);

// Clears all of the data from the LCD
void lcd_clear(void);

// Sets the cursor back to its home position on the LCD
void lcd_cursor_home(void);

// Hides the blinking cursor on the LCD
void lcd_cursor_set(int x, int y);

// Sets the cursor to a specific (x, y) position on the LCD
void lcd_cursor_show(void);

// Shows the blinking cursor on the LCD
void lcd_cursor_hide(void);

// Prints a formatted string to the LCD
void lcd_printf(const char * format, ...);
