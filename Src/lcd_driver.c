// file: lcd_driver.c
// created by: Grant Wilk
// date created: 12/17/2019
// last modified: 1/5/2020
// description: Contains functions for driving the LCD on the CE development board

# include <stdio.h>
# include <stdarg.h>
# include <stdint.h>
# include "delay.h"
# include "lcd_driver.h"

// RCC Addresses
# define RCC_BASE 0x40023800
# define RCC_AHB1ENR (RCC_BASE + 0x0030)

// RCC Values
# define RCC_GPIOAEN (1 << 0)
# define RCC_GPIOCEN (1 << 2)

// GPIOA Addresses
# define GPIOA_BASE 0x40020000
# define GPIOA_MODER (GPIOA_BASE + 0x00)
# define GPIOA_ODR (GPIOA_BASE + 0x14)

// GPIOA Values
# define GPIOA_ODR_LCD_DATABUS 0xFF0
# define GPIOA_MODER_LCD_DATABUS_OUTPUT 0x555500

// GPIOC Addresses
# define GPIOC_BASE 0x40020800
# define GPIOC_MODER (GPIOC_BASE + 0x00)
# define GPIOC_ODR (GPIOC_BASE + 0x14)

// GPIOC Values
# define GPIOC_ODR_LCD_RS (1 << 8)
# define GPIOC_ODR_LCD_RW (1 << 9)
# define GPIOC_ODR_LCD_E (1 << 10)
# define GPIOC_MODER_LCD_RS_OUTPUT (1 << 16)
# define GPIOC_MODER_LCD_RW_OUTPUT (1 << 18)
# define GPIOC_MODER_LCD_E_OUTPUT (1 << 20)

// Other Values
# define DATABUS_MAX_VALUE 0xFF

// LCD Characteristics
# define LCD_ROW_LENGTH 40
# define LCD_MAX_LENGTH 80

// Static Function Prototypes
static void lcd_print_string(char s[]);
static void lcd_write_instruction(int instruction);
static void lcd_write_char(char character);
static void lcd_instr_clear(void);
static void lcd_instr_return_home(void);
static void lcd_instr_entry_mode_set(int cursorDirection, int displayShift);
static void lcd_instr_display_on_off(int displayOn, int cursorOn, int cursorPosOn);
static void lcd_instr_cursor_display_shift(int shift, int direction);
static void lcd_instr_function_set(int dataInterface, int lineNumber, int fontSize);

// Global Variables
static uint32_t * const gpioaODR = (uint32_t *) GPIOA_ODR;
static uint32_t * const gpiocODR = (uint32_t *) GPIOC_ODR;

// Initializes the LCD pins and readies the LCD peripheral for use
// @ param void
// @ return void
void lcd_init(void) {
    // enable GPIOA and GPIOC in RCC
    uint32_t * rccAHB1ENR = (uint32_t *) RCC_AHB1ENR;
    * rccAHB1ENR |= RCC_GPIOAEN | RCC_GPIOCEN;

    // set LCD databus pins as outputs
    uint32_t * gpioaMODER = (uint32_t *) GPIOA_MODER;
    * gpioaMODER |= GPIOA_MODER_LCD_DATABUS_OUTPUT;

    // set LCD control pins as outputs
    uint32_t * gpiocMODER = (uint32_t *) GPIOC_MODER;
    * gpiocMODER |= GPIOC_MODER_LCD_RS_OUTPUT | GPIOC_MODER_LCD_RW_OUTPUT | GPIOC_MODER_LCD_E_OUTPUT;

    // function set 8-bit interface, 1 line, and 5x8 font size
    lcd_instr_function_set(1, 1, 0);

    // function set 8-bit interface, 1 line, and 5x8 font size
    lcd_instr_function_set(1, 1, 0);

    // turn on display and cursor position
    lcd_instr_display_on_off(1, 0, 1);

    // clear the display
    lcd_clear();

    // entry mode set increment position, display shift off
    lcd_instr_entry_mode_set(1, 0);
}

// Clears the display of the LCD
// @ param void
// @ return void
void lcd_clear(void) {
    lcd_instr_clear();
}

// Moves the cursor back to it's home position
// @ param void
// @ return void
void lcd_cursor_home(void) {
    lcd_instr_return_home();
}

// Sets the cursor to a specific (x, y) position on the LCD
// @ param x - the zero-based x-position
// @ param y - the zero-based y-position
// @ return void
void lcd_cursor_set(int x, int y) {

    // move the cursor back to its home position
    lcd_cursor_home();

    // move the cursor a full row length for each value of y
    for (int i = 0; i < y * LCD_ROW_LENGTH; i++) {
        lcd_instr_cursor_display_shift(0, 1);
    }

    // move the cursor over a single column for each value of x
    for (int i = 0; i < x; i++) {
        lcd_instr_cursor_display_shift(0, 1);
    }
}

// Shows the blinking cursor on the LCD
// @ param void
// @ return void
void lcd_cursor_show(void) {
    lcd_instr_display_on_off(1, 0, 1);
}

// Hides the blinking cursor on the LCD
// @ param void
// @ return void
void lcd_cursor_hide(void) {
    lcd_instr_display_on_off(1, 0, 0);
}

// Prints a formatted string to the LCD
// @ param format - a variable length argument
// @ return void
void lcd_printf(const char * format, ... ) {

    // initialize the variable arg list
    va_list args;
    va_start(args, format);

    // declare the buffer that will store the formatted string
    char buffer [LCD_MAX_LENGTH];

    // print to the string buffer
    vsprintf(buffer, format, args);

    // print the string buffer to the LCD
    lcd_print_string(buffer);

    // terminate the variable arg list
    va_end(args);

}

// Prints a string to the LCD
// @ param s - the string to print
// @ return void
static void lcd_print_string(char s[]) {

    int offset = 0;

    // print every character in the string
    while (s[offset] != '\0') {
        lcd_write_char(s[offset++]);
    }

}

// Writes an instruction to the LCD
// @ param instruction - the hexadecimal instruction to write to the LCD
// @ return void
static void lcd_write_instruction(int instruction) {

    // only write the instruction if it fits in the databus (less than or equal to 0xFF)
    if (instruction <= DATABUS_MAX_VALUE) {

        // set E
        * gpiocODR |= GPIOC_ODR_LCD_E;

        // clear RS and RW
        * gpiocODR &= ~(GPIOC_ODR_LCD_RS | GPIOC_ODR_LCD_RW);

        // clear databus pins and copy instruction to them
        * gpioaODR &= ~(GPIOA_ODR_LCD_DATABUS);
        * gpioaODR |= (instruction << 4);

        // clear E to write the instruction
        * gpiocODR &= ~(GPIOC_ODR_LCD_E);
    }
}

// Writes a character to the LCD
// @ param character - the character to write to the LCD
// @ return void
static void lcd_write_char(char character) {
    // only write the character if it fits in the databus (less than or equal to 0xFF)
    if (character <= DATABUS_MAX_VALUE) {

        // set E and RS
        * gpiocODR |= GPIOC_ODR_LCD_E | GPIOC_ODR_LCD_RS;

        // clear RW
        * gpiocODR &= ~(GPIOC_ODR_LCD_RW);

        // clear databus pins and copy character to them
        * gpioaODR &= ~(GPIOA_ODR_LCD_DATABUS);
        * gpioaODR |= (character << 4);

        // clear E to write the instruction
        * gpiocODR &= ~(GPIOC_ODR_LCD_E);

        // delay for 10us
        delay_us(37);
    }
}

// Clear display instruction for the LCD
// @ param void
// @ return void
static void lcd_instr_clear(void) {
    // the clear display instruction
    int instruction = (1 << 0);

    // write the instruction
    lcd_write_instruction(instruction);

    // delay
    delay_us(1520);
}

// Return home instruction for the LCD
// @ param void
// @ return void
static void lcd_instr_return_home(void) {
    // the return home instruction
    int instruction = (1 << 1);

    // write the instruction
    lcd_write_instruction(instruction);

    // delay
    delay_us(1520);
}

// Entry mode set instruction for the LCD
// @ param cursorDirection - when data is written, increment the cursor position if 1, decrement the cursor position if 0
// @ param displayShift - shift enabled if 1, shift disabled if 0
// @ return void
static void lcd_instr_entry_mode_set(int cursorDirection, int displayShift) {

    // the base entry mode set instruction
    int instruction = (1 << 2);

    // set instruction parameters based on function parameters
    if (cursorDirection) instruction |= (1 << 1);
    if (displayShift) instruction |= (1 << 0);

    // write instruction
    lcd_write_instruction(instruction);

    // delay
    delay_us(37);
}

// Display ON/OFF instruction for the LCD
// @ param displayOn - entire display on
// @ param cursorOn - show cursor (underscore)
// @ param cursorBlinkOn - show cursor blink (block)
// @ return void
static void lcd_instr_display_on_off(int displayOn, int cursorOn, int cursorBlinkOn) {

    // the base display on/off instruction
    int instruction = (1 << 3);

    // set instruction parameters based on function parameters
    if (displayOn) instruction |= (1 << 2);
    if (cursorOn) instruction |= (1 << 1);
    if (cursorBlinkOn) instruction |= (1 << 0);

    // write the instruction
    lcd_write_instruction(instruction);

    // delay
    delay_us(37);
}

// Cursor display/shift instruction for the LCD
// @ param shiftSelect - shift the display if 1, shift the cursor if 0
// @ param direction - right if 1, left if 0
// @ return void
static void lcd_instr_cursor_display_shift(int shiftSelect, int direction) {
    // the base cursor display/shift instruction
    int instruction = (1 << 4);

    // set instruction parameters based on function parameters
    if (shiftSelect) instruction |= (1 << 3);
    if (direction) instruction |= (1 << 2);

    // write instruction
    lcd_write_instruction(instruction);

    // delay
    delay_us(37);
}

// Function set instruction for the LCD
// @ param dataInterface - 8-bit interface if 0, 4-bit interface if 1
// @ param lineNumber - line number 2 if 0, line number 1 if 1
// @ param fontSize - font size 5x11 if 0, font size 5x8 if 1
// @ return void
static void lcd_instr_function_set(int dataInterface, int lineNumber, int fontSize) {

    // the base function set instruction
    int instruction = (1 << 5);

    // set instruction parameters based on function parameters
    if (dataInterface) instruction |= (1 << 4);
    if (lineNumber) instruction |= (1 << 3);
    if (fontSize) instruction |= (1 << 2);

    // write the instruction
    lcd_write_instruction(instruction);

    // delay
    delay_us(37);
}
