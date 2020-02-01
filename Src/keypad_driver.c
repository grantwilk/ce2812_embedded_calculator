// file: keypad_driver.c
// created by: Grant Wilk
// date created: 1/5/2020
// last modified: 1/5/2020
// description: Contains functions for driving the keypad on the CE development board

# include <stdint.h>
# include "delay.h"
# include "keypad_driver.h"
# include "lcd_driver.h"

// RCC Addresses
# define RCC_BASE 0x40023800
# define RCC_AHB1ENR (RCC_BASE + 0x30)
# define RCC_APB2ENR (RCC_BASE + 0x44)

// RCC Values
# define RCC_AHB1ENR_GPIOCEN (1 << 2)
# define RCC_APB2ENR_SYSCFGEN (1 << 14)

// GPIOC Addresses
# define GPIOC_BASE 0x40020800
# define GPIOC_MODER (GPIOC_BASE + 0x00)
# define GPIOC_PUPDR (GPIOC_BASE + 0x0C)
# define GPIOC_IDR (GPIOC_BASE + 0x10)
# define GPIOC_ODR (GPIOC_BASE + 0x14)

// GPIOC Values
# define GPIOC_COLUMNS 0xFF
# define GPIOC_ROWS 0xFF00
# define GPIOC_MODER_COLUMNS_OUTPUT 0x55
# define GPIOC_MODER_ROWS_OUTPUT 0x5500
# define GPIOC_ODR_COLUMNS 0x0F
# define GPIOC_ODR_ROWS 0xF0
# define GPIOC_PUPDR_COLUMNS_PULLDOWN 0xAA
# define GPIOC_PUPDR_ROWS_PULLDOWN 0xAA00

// SYSCFG Addresses
# define SYSCFG_BASE 0x40013800
# define SYSCFG_EXTICR1 (SYSCFG_BASE + 0x08)

// SYSCFG Values
# define SYSCFG_EXTIX_TO_PIN_C (1 << 1)

// EXTI Addresses
# define EXTI_BASE 0x40013C00
# define EXTI_IMR (EXTI_BASE + 0x00)
# define EXTI_RTSR (EXTI_BASE + 0x08)
# define EXTI_PR (EXTI_BASE + 0x14)

// EXTI Values
# define EXTI_0_THRU_4 0x0F

// NVIC Addresses
# define NVIC_BASE 0xE000E100
# define NVIC_ISER0 (NVIC_BASE + 0x00)
# define NVIC_ICER0 (NVIC_BASE + 0x80)

// NVIC Values
# define NVIC_6_THRU_9 (0b1111 << 6)

// Register Pointers
static uint32_t * const gpiocMODER = (uint32_t *) GPIOC_MODER;
static uint32_t * const gpiocIDR = (uint32_t *) GPIOC_IDR;

// Row Lookup Table
const static int rowLUT[9] = {0, 0, 1, 1, 2, 2, 2, 2, 3};

// Character Lookup Table
const static char defaultCharLUT[17] =
{
	'\0',
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

// Character Lookup Table Pointer
static char * charLUT = (char *) defaultCharLUT;

// Last Keypress Variable
static char lastKeypress = 0;

// Initializes the keypad pins and readies the keypad peripheral for use
// @ param void
// @ return void
void key_init(void) {

    // enable GPIOC in RCC
    uint32_t * rccAHB1ENR = (uint32_t *) RCC_AHB1ENR;
    * rccAHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    // set ODR to output 1's for both rows and columns
    uint32_t * gpiocODR = (uint32_t *) GPIOC_ODR;
    * gpiocODR |= GPIOC_ODR_COLUMNS | GPIOC_ODR_ROWS;

    // set PUPDR as pull-down for both rows and columns
    uint32_t * gpiocPUPDR = (uint32_t *) GPIOC_PUPDR;
    * gpiocPUPDR &= ~(GPIOC_ROWS | GPIOC_COLUMNS);
    * gpiocPUPDR |= (GPIOC_PUPDR_COLUMNS_PULLDOWN | GPIOC_PUPDR_ROWS_PULLDOWN);

    // configure the columns as inputs and rows as outputs
    * gpiocMODER &= ~(GPIOC_COLUMNS | GPIOC_ROWS);
    * gpiocMODER |= (GPIOC_MODER_ROWS_OUTPUT);

    // enable SYSCFG in RCC
    uint32_t * rccAPB2ENR = (uint32_t *) RCC_APB2ENR;
    * rccAPB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // map EXTI to pins on GPIOC
    uint32_t * syscfgEXTICR1 = (uint32_t *) SYSCFG_EXTICR1;
    * syscfgEXTICR1 |= (SYSCFG_EXTIX_TO_PIN_C);
    * syscfgEXTICR1 |= (SYSCFG_EXTIX_TO_PIN_C << 4);
    * syscfgEXTICR1 |= (SYSCFG_EXTIX_TO_PIN_C << 8);
    * syscfgEXTICR1 |= (SYSCFG_EXTIX_TO_PIN_C << 12);

    // unmask EXTI0-EXTI3 in EXTI IMR
    uint32_t * extiIMR = (uint32_t *) EXTI_IMR;
    * extiIMR |= EXTI_0_THRU_4;

    // set interrupts on rising edge for EXTI0-EXTI3 in EXTI RTSR
    uint32_t * extiRTSR = (uint32_t *) EXTI_RTSR;
    * extiRTSR |= EXTI_0_THRU_4;

    // enable interrupt in NVIC
    uint32_t * nvicISER0 = (uint32_t *) NVIC_ISER0;
    * nvicISER0 = NVIC_6_THRU_9;

    // clear the last keypress from memory
    key_clear();

}

// Clears the last key pressed
// @ param void
// @ return void
void key_clear(void) {
	lastKeypress = 0;
}

// Blocks program flow and waits for a keypress
// @ param void
// @ return void
void key_wait(void) {
	key_clear();
    while (key_get() == 0);
}

// Gets the last key pressed and returns it
// @ param void
// @ return the last key pressed or 0 if no key was pressed
int key_get(void) {
	return lastKeypress;
}

// Blocks program flow, waits for a keypress, and returns it
// @ param void
// @ return the last key pressed
int key_get_wait(void) {
    key_wait();
    return key_get();
}

// Gets the last key pressed, converts it to a character, and returns it
// @ param void
// @ return the character of the last key pressed or 0 if no key was pressed
char key_get_char(void) {
    return charLUT[key_get()];
}

// Blocks program flow, waits for a keypress, converts it to a character, and returns it
// @ param void
// @ return the last key pressed
char key_get_char_wait(void) {
    return charLUT[key_get_wait()];
}


// Converts a keypress to a character and returns it
// @ param key - the number of a keypress
// @ return the character associated with the keypress if it is in the range of valid characters, otherwise return a null pointer
char key_to_char(int key) {
	if (key > 0 && key <= 16) {
		return charLUT[key];
	} else {
		return '\0';
	}
}

// Sets a new character LUT for get character functions
// @ param newCharLUT - the new character LUT, a 17 element character array that begins with a null terminator character
// @ return void
void key_set_char_lut(char newCharLUT[]) {
	charLUT = newCharLUT;
}

// Handles keypad interrupts
// @ param column - the column the interrupt occurred on
// @ return void
static void key_interrupt_handler(int column) {

    // mask EXTI0-EXTI3 in EXTI IMR
    uint32_t * extiIMR = (uint32_t *) EXTI_IMR;
    * extiIMR &= ~(EXTI_0_THRU_4);

    // set rows as inputs and columns as outputs
    * gpiocMODER &= ~(GPIOC_COLUMNS | GPIOC_ROWS);
    * gpiocMODER |= (GPIOC_MODER_COLUMNS_OUTPUT);

    // delay 40 milliseconds for debouncing
    delay_ms(40);

    // get the one-hot value of the row
    int row = (* gpiocIDR >> 4) & 0xF;

    // if a key is still pressed and the keypress is in a valid position
    if (row > 0 && row <= 9) {

        // get the actual value of the row from the LUT
        row = rowLUT[row];

        // update the last keypress variable
        lastKeypress = row * 4 + column + 1;
    }

    // set rows as outputs and columns as inputs
    * gpiocMODER &= ~(GPIOC_COLUMNS | GPIOC_ROWS);
    * gpiocMODER |= (GPIOC_MODER_ROWS_OUTPUT);

    // clear the pending interrupt
    uint32_t * extiPR = (uint32_t *) EXTI_PR;
    * extiPR = (1 << column);

    // unmask EXTI0-EXTI3 in EXTI IMR
    * extiIMR |= EXTI_0_THRU_4;

}

// Keypad column 0 interrupt handler
// @ param void
// @ return void
void EXTI0_IRQHandler(void) {
	key_interrupt_handler(0);
}

// Keypad column 1 interrupt handler
// @ param void
// @ return void
void EXTI1_IRQHandler(void) {
	key_interrupt_handler(1);
}

// Keypad column 2 interrupt handler
// @ param void
// @ return void
void EXTI2_IRQHandler(void) {
	key_interrupt_handler(2);
}

// Keypad column 3 interrupt handler
// @ param void
// @ return void
void EXTI3_IRQHandler(void) {
	key_interrupt_handler(3);
}
