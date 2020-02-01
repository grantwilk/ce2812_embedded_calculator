// file: keypad_driver.h
// created by: Grant Wilk
// date created: 1/5/2020
// last modified: 1/5/2020
// description: Header file for keypad_driver.c

// Initializes the keypad pins and readies the keypad peripheral for use
void key_init(void);

// Clears the last key pressed
void key_clear(void);

// Blocks program flow and waits for a keypress
void key_wait(void);

// Gets the last key pressed and returns it
int key_get(void);

// Blocks program flow, waits for a keypress, and returns it
int key_get_wait(void);

// Gets the last key pressed, converts it to a character, and returns it
char key_get_char(void);

// Blocks program flow, waits for a keypress, converts it to a character, and returns it
char key_get_char_wait(void);

// Converts a keypress to a character and returns it
char key_to_char(int key);

// Sets a new character LUT for get character functions
void key_set_char_lut(char * newCharLUT);
