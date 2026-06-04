#ifndef ARGS_H
#define ARGS_H

#define DRIVE_LETTER_LEN 16
#define PATH_LEN 512
#define FILENAME_LEN 256
#define TOTAL_PATH_LEN DRIVE_LETTER_LEN + PATH_LEN + FILENAME_LEN

typedef struct
{
    char *input_file;
    char *output_file;
    int offset;
    int interleave;
    int channels;
    int chunks;
    int frequency;
} adpcm_parameters;

/**
 * Read parameters and store the necessary values into specified struct
 *
 * Includes data validation, warnings and errors. The modified struct has all
 * necessary data for adpcm conversion.
 *
 * @param argc number of arguments
 * @param argv arguments
 * @param params pointer to adpcm_parameters struct, where arguments are stored
 *
 * @retval 0 on success
 * @retval -EINVAL if any of arguments has invalid data
 */
int read_param(int argc, const char **argv, adpcm_parameters *params);

/**
 * Create a path for output file
 * If input file path is /a/b/c/d.ext then the output file path will be
 * /a/b/c/d.wav
 *
 * @warning this is not a safe operation and is only ment for usage in
 * read_param() function, due to there not being any buffer size checks.
 *
 * @param input_path input path string
 * @param output_path pointer to output path pointer
 *
 * @retval 0 on success
 */
int create_output_file_path(char *input_path, char *output_path);

#endif
