#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "print_conv.h"
#include "args.h"

#ifdef __unix__
#include <libgen.h>
#endif

int read_param(int argc, char *argv[], adpcm_parameters *params)
{
    int offset = 0;
    int interleave = 16;
    int chunks = 0;
    int channels = 1;
    int frequency = 0;
    char *input_path = (char *)calloc(TOTAL_PATH_LEN, sizeof(char));
    char *output_path = (char *)calloc(TOTAL_PATH_LEN, sizeof(char));

    // For correct freeing of memory
    params->input_file = input_path;
    params->output_file = output_path;

    int opt = -1;
    int ret = 0;

    while ((opt = getopt(argc, argv, "i:s:c:l:n:o:f:h")) != -1)
    {
        switch (opt)
        {
        case 'i':
            strncpy(input_path, optarg, TOTAL_PATH_LEN);
            break;
        case 's':
            if (str_to_int(optarg, &offset))
            {
                print_error("Invalid argument for offset.");
                ret = -EINVAL;
                goto error;
            }
            break;
        case 'c':
            if (str_to_int(optarg, &channels))
            {
                print_error("Invalid argument for channels.");
                ret = -EINVAL;
                goto error;
            }
            break;
        case 'l':
            if (str_to_int(optarg, &interleave))
            {
                print_error("Invalid argument for interleave.");
                ret = -EINVAL;
                goto error;
            }
            break;
        case 'n':
            if (str_to_int(optarg, &chunks))
            {
                print_error("Invalid argument for chunks.");
                ret = -EINVAL;
                goto error;
            }
            break;
        case 'o':
            // output_path = strcpy(optarg);
            strncpy(output_path, optarg, TOTAL_PATH_LEN);
            break;
        case 'f':
            if (str_to_int(optarg, &frequency))
            {
                print_error("Invalid argument for frequency.");
                ret = -EINVAL;
                goto error;
            }
            break;            
        default:
            printf(
                "Convert 4-bit PlayStation ADPCM to wav file.\n"
                "\n"
                "Usage:\n"
                "    psxadec -i <in> [options] -f <frequency>\n"
                "\n"
                "Options:\n"
                "  Required:\n"
                "    -i          Input file path\n"
                "    -f          Audio frequency (in hz)\n"
                "  Optional:\n"
                "    -s          Header skip / Data beginning offset (default 0)\n"
                "    -c          Number of channels (default 1)\n"
                "    -l          Interleave between channels (default 16)\n"
                "    -n          Number of chunks to be read (default 0 ie all)\n"
                "    -o          Output file path (default is input file path with replaced wav extension)\n"
                "\n"
                "Example usages:\n"
                "    psxadec -h                       Print this help dialog\n"
                "    psxadec -i ./file.vag -f 44100\n"
                "    psxadec -i ./file.vag -c 2 -s 0x4000 -l 0x8000 -f 48000\n"
                "    psxadec -i ./file.vag -c 1 -s 32 -f 22500 -o ./file.wav\n"
            );
            ret = (opt == 'h') ? 1 : -EINVAL;
            goto error;
        }
    }

    /**
     * Input error handling
     */
    if (strnlen(input_path, 2) == 0)
    {
        print_error("Input path not present\n");
        ret = -EINVAL;
        goto error;
    }
    if (offset < 0)
    {
        print_warning("Offset should be an integer greater than or equal to 0. Defaulting to 0.\n");
        offset = 0;
    }
    if (channels < 1)
    {
        print_warning("Number of channels should be an integer greater than or equal to 1. Defaulting to 1.\n");
        channels = 1;
    }
    if (channels >= 2 && interleave <= 0)
    {
        print_error("Multichannel audio has to have interleave.\n");
        ret = -EINVAL;
        goto error;
    }
    if (frequency <= 0)
    {
        print_error("Frequency has to be a positive number\n");
        ret = -EINVAL;
        goto error;
    }
    if (interleave <= 0)
    {
        print_warning("Interleave should be positive. Defaulting to 16.\n");
        interleave = 16;
    }
    if (channels == 1 && interleave != 16)
    {
        print_warning("Mono audio always uses interleave 16.\n");
        interleave = 16;
    }
    if (channels >= 2 && interleave < 0x1000)
    {
        print_warning("For audio with 2+ channels the current interleave value seems too small.\n");
    }
    if (chunks < 0)
    {
        print_warning("Chunks should be an integer greater than or equal to 1. Defaulting to 0 (Read all chunks).\n");
        chunks = 0;
    }

    // No output path given, try to create one
    if (strnlen(output_path, 2) == 0)
    {
        ret = create_output_file_path(input_path, output_path);
    }
    if (ret)
    {
        goto error;
    }

    params->channels = channels;
    params->chunks = chunks;
    params->interleave = interleave;
    params->offset = offset;
    params->frequency = frequency;

error:
    return ret;
}

int create_output_file_path(char *input_path, char *output_path)
{
    int ret = 0;

#ifdef _WIN32
    char drive[DRIVE_LETTER_LEN];
    char directory[PATH_LEN];
    char filename[FILENAME_LEN];
    ret = _splitpath_s(input_path, drive, DRIVE_LETTER_LEN, directory, PATH_LEN,
                       filename, FILENAME_LEN, NULL, 0);
    strcpy(output_path, drive);     // Copy over drive name
    strcat(output_path, directory); // Add directory
    strcat(output_path, filename);  // Add filename
    strcat(output_path, ".wav");    // Add extension

    return ret;
#elif defined __unix__
    char *directory;
    char *bname;
    char temp_input_path[FILENAME_LEN];  // Because libgen's basename modifies the original array
    char filename[FILENAME_LEN];

    strncpy(temp_input_path, input_path, FILENAME_LEN);

    bname = basename(temp_input_path);
    directory = dirname(temp_input_path);

    // Remove extension if there is one
    char *last_dot = strrchr(bname, '.');
    if (last_dot == NULL)
    {
        strncpy(filename, bname, FILENAME_LEN);
    }
    else
    {
        long filename_length = last_dot - bname;  // Not good
        strncpy(filename, bname, FILENAME_LEN > filename_length ? filename_length : FILENAME_LEN);
    }

    strncpy(output_path, directory, PATH_LEN);
    strcat(output_path, "/");
    strncat(output_path, filename, FILENAME_LEN);
    strcat(output_path, ".wav");

    return ret;
#endif
}

void free_param_struct(adpcm_parameters *parameters)
{
    if (parameters == NULL)
    {
        return;
    }
    if (parameters->input_file != NULL)
    {
        free(parameters->input_file);
        parameters->input_file = NULL;
    }
    if (parameters->output_file != NULL)
    {
        free(parameters->output_file);
        parameters->output_file = NULL;
    }
    free(parameters);
    parameters = NULL;
}
