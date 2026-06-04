#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "print_conv.h"

void print_error(char *message)
{
    fprintf(stderr, "\033[91m[ERROR]\033[0m %s", message);
}

void print_warning(char *message)
{
    printf("\033[93m[WARNING]\033[0m %s", message);
}


