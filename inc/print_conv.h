#ifndef COL_PRINT_H
#define COL_PRINT_H

#define MAX_NUMBER_LENGTH 10

/**
 * Print out an error
 *
 * @param message pointer to message string
 */
void print_error(char *message);

/**
 * Print out a warning
 *
 * @param message pointer to string
 */
void print_warning(char *message);

/**
 * Convert string to integer
 *
 * Takes in positive hexadecimal or decimal integer and converts into integer
 * Inputted string can only include number, except hexadecimal integer, that
 * has to have a prefix 0x followed by the numbers / letters
 *
 * @param input pointer to input string
 * @param output pointer to output integer
 * @return
 * -EINVAL if given string is not
 * 0 if conversion was successful
 */
int str_to_int(char *input, int *output);

#endif
