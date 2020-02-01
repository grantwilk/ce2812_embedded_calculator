/*
In this lab we were assigned the task of creating an LCD driver, a keypad driver, and a demonstrative application of our own design. This lab helped me reaffirm my understanding of how our development board's LCD interface works, gave me a significant amount of experience working with C-style strings, and allowed me to experiment a bit on my own (which is always appreciated).

I also took a few creative liberties with the LCD and keypad API. In the LCD API, I implemented a lcd_printf() function instead of lcd_print_string() and lcd_print_num(). I repurposed the lcd_print_string() as a helper function. I built the keypad API from the ground up because I wanted an interrupt driven keypad instead of a polling one. I also took the time to make some additional functions including a function for replacing the character lookup table, wait functions, conversion tools, and more.
*/

// file: main.c
// created by: Grant Wilk
// date created: 12/17/2019
// last modified: 1/11/2020
// description: A calculator program with overflow and divide by zero protection

# include <stdio.h>
# include <stdlib.h>
# include <limits.h>
# include "delay.h"
# include "lcd_driver.h"
# include "keypad_driver.h"

int main(void) {

	// initialize peripherals
	key_init();
	lcd_init();

	// op string contains the first operand, operator, and second operand terminated with a null terminator
	char opString[33];
	volatile int opStringLength = 0;

	// operand lengths
	int firstOperandLength = 0;
	int secondOperandLength = 0;

	// operand and operator flags
	char operatorEntered = 0;
	char secondOperandEntered = 0;
	char resultDisplayed = 0;

	while (1) {

		// block program flow and wait for a keypress from the keypad
		int key = key_get_wait();

		// if a number key is pressed
		if ((key >= 1 && key < 4) || (key >= 5 && key < 8) || (key >= 9 && key < 12) || (key == 14)) {

			// do not accept new number inputs if the result is being displayed
			if (!resultDisplayed) {

				// do not accept new number inputs if the respective operand is longer than 9 digits
				if ((!operatorEntered && firstOperandLength < 9) || (operatorEntered && secondOperandLength < 9)) {

					// if no operator has been entered, increment the first operand length
					if (!operatorEntered) firstOperandLength++;

					// if an operator has been entered, increment the second operand length and set the second operand entered flag
					if (operatorEntered) {
						secondOperandLength++;
						secondOperandEntered = 1;
					}

					// get the character associated with the key
					char key_char = key_to_char(key);

					// add the character to the op string
					opString[opStringLength++] = key_char;

					// print the character to the LCD
					lcd_printf("%c", key_char);

				}

			}

		// if an operator key is pressed
		} else if ((key == 4) || (key == 8) || (key == 12) || (key == 16)) {

			// as long as there is some sort of input
			if (opStringLength != 0 && !secondOperandEntered) {

				char opChar;

				// set operator entered flag
				operatorEntered = 1;

				// clear result displayed flag
				resultDisplayed = 0;

				// add the operator to the op string and print it to the LCD
				switch (key) {

					// add key pressed
					case 4:
						opChar = '+';
						break;

					// subtract key pressed
					case 8:
						opChar = '-';
						break;

					// multiply key pressed
					case 12:
						opChar = '*';
						break;

					// divide key pressed
					case 16:
						opChar = '/';
						break;

					// unknown key pressed (should never occur)
					default:
						break;

				}

				// append the operator character to the op string
				opString[opStringLength++] = opChar;

				// move the cursor to the top right corner of the LCD
				lcd_cursor_set(15, 0);

				// print the operator character to the LCD
				lcd_printf("%c", opChar);

				// move the cursor to the bottom left corner of the LCD
				lcd_cursor_set(0, 1);

			}

		// if the equals key is pressed
		} else if (key == 15) {

			// if the second operand has been entered
			if (secondOperandEntered) {

				// add a null terminator to the end of the op string
				opString[opStringLength++] = '\0';

				// parse the op string
				int firstOperand;
				char operatorChar;
				int secondOperand;

				sscanf(opString, "%d%c%d", &firstOperand, &operatorChar, &secondOperand);

				// overflow and underflow flags
				char overflow;
				char underflow;

				// do the calculation
				int result;

				switch (operatorChar) {

					// add operator
					case '+':;

						// determine if addition will overflow or underflow
						overflow = (secondOperand > 0) && (firstOperand > INT_MAX - secondOperand);
						underflow = (secondOperand < 0) && (firstOperand < INT_MIN - secondOperand);

						// default to zero if overflow/underflow, otherwise complete the calculation
						if (overflow || underflow) {
							result = 0;
						} else {
							result = firstOperand + secondOperand;
						}

						break;

					// subtract operator
					case '-':;

						// determine if subtraction will overflow or underflow
						overflow = (secondOperand < 0) && (firstOperand > INT_MAX + secondOperand);
						underflow = (secondOperand > 0) && (firstOperand < INT_MIN + secondOperand);

						// default to zero if overflow/underflow, otherwise complete the calculation
						if (overflow || underflow) {
							result = 0;
						} else {
							result = firstOperand - secondOperand;
						}

						break;

					// multiply operator
					case '*':;

						// determine if multiplication will overflow or underflow
						overflow = firstOperand > INT_MAX / secondOperand;
						underflow = firstOperand < INT_MIN / secondOperand;

						// default to zero if overflow/underflow, otherwise complete the calculation
						if (overflow || underflow) {
							result = 0;
						} else {
							result = firstOperand * secondOperand;
						}

						break;

					// divide operator
					case '/':;

						// default to zero if dividing by zero, otherwise complete the calculation
						if (secondOperand == 0) {
							result = 0;
						} else {
							result = firstOperand / secondOperand;
						}

						break;

					// unknown operator, default to 0
					default:
						result = 0;

				}

				// clear the LCD
				lcd_clear();

				// print result to the LCD
				lcd_printf("%d", result);

				if (result == 69) {

					lcd_cursor_hide();
					delay_ms(1000);
					lcd_printf(" ");

					for (int i = 0; i < 3; i++) {
						delay_ms(150);
						lcd_printf(".");
					}

					delay_ms(800);
					lcd_printf(" nice.");

					delay_ms(1000);
					lcd_cursor_show();
				}

				// copy the result back into the op string for chained calculations
				sprintf(opString, "%d", result);

				// move the op string length pointer to the end of the first operand
				opStringLength = 0;
				while (opString[opStringLength++ + 1] != '\0');

				// update operand lengths
				firstOperandLength = opStringLength;
				secondOperandLength = 0;

				// update flags
				operatorEntered = 0;
				secondOperandEntered = 0;
				resultDisplayed = 1;

			}

		// if the clear key is pressed
		} else {

			// reset op string length
			opStringLength = 0;

			// reset operand lengths
			firstOperandLength = 0;
			secondOperandLength = 0;

			// reset operand entered flags
			operatorEntered = 0;
			secondOperandEntered = 0;
			resultDisplayed = 0;

			// clear the LCD
			lcd_clear();

		}

	}

}
