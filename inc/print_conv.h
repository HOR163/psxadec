#ifndef PRINT_CONV_H
#define PRINT_CONV_H

#define print_err(fmt, ...) fprintf(stderr, "\033[91m[ERROR]\033[0m " fmt, ##__VA_ARGS__)
#define print_wrn(fmt, ...) fprintf(stdout, "\033[93m[WARNING]\033[0m " fmt, ##__VA_ARGS__)

#endif
