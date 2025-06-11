#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "print_conv.h"

void print_error(char *message)
{
    fprintf(stderr, "\033[91m[ERROR]\033[0m %s", message);
}

void print_warning(char *message)
{
    printf("\033[93m[WARNING]\033[0m %s", message);
}

int str_to_int(char *input, int *output)
{
    /**
     * Check for hexadecimal numbers
     */
    char starts0x[3] = {'\02', '0', 'x'}; // First one is length of the string
    if (strncmp(input, (starts0x + 1), *starts0x) == 0)
    {
        *output = (int)strtol(input, NULL, 16);
        // Check that the lengths also match
        char hex_str[MAX_NUMBER_LENGTH];
        snprintf(hex_str, MAX_NUMBER_LENGTH, "%x", *output);
        if (strnlen(input, MAX_NUMBER_LENGTH) == strnlen(hex_str, MAX_NUMBER_LENGTH) + 2)
        {
            return 0;
        }
        return -EINVAL;
    }

    /**
     * If it wasn't a hexadecimal, then it's a decimal number
     */
    *output = atoi(input); // Takes care of the sign as well

    // Check that the input length matches to the converted integer's length
    char int_str[MAX_NUMBER_LENGTH];
    snprintf(int_str, MAX_NUMBER_LENGTH, "%d", *output);
    if (strnlen(int_str, MAX_NUMBER_LENGTH) == strnlen(input, MAX_NUMBER_LENGTH))
    {
        return 0;
    }
    return -EINVAL;
}