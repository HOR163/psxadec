#include <errno.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "argparse.h"
#include "args.h"
#include "print_conv.h"

#ifdef __unix__
#include <libgen.h>
#endif

static char fallback_output_path[TOTAL_PATH_LEN] = {0};

static const char *const usages[] = {
    "psxadec -i file -f frequency [-s skip] [-c channels] [-l interleave] [-n chunks] [-o file]",
    "psxadec -i ./file.vag -f 44100",
    "psxadec -i ./file.vag -c 2 -s 0x4000 -l 0x8000 -f 48000",
    "psxadec -i ./file.vag -c 1 -s 32 -f 22500 -o ./file.wav",
    NULL,
};

int read_param(int argc, const char **argv, adpcm_parameters *params)
{
    int ret = 0;

    int offset = 0;
    int interleave = 16;
    int chunks = 0;
    int channels = 1;
    int frequency = 0;
    char *input_path = NULL;
    char *output_path = NULL;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('i', "input", &input_path, "[REQUIRED] Input file path", NULL, 0, 0),
        OPT_INTEGER('f', "frequency", &frequency, "[REQUIRED] Audio frequency (in hz)", NULL, 0, 0),
        OPT_INTEGER('s', "skip", &offset, "Header skip / Data beginning offset (default 0)", NULL, 0, 0),
        OPT_INTEGER('c', "channels", &channels, "Number of channels (default 1)", NULL, 0, 0),
        OPT_INTEGER('l', "interleave", &interleave, "Interleave between channels (default 16)", NULL, 0, 0),
        OPT_INTEGER('n', "chunks", &chunks, "Number of chunks to be read (default 0 ie all)", NULL, 0, 0),
        OPT_STRING('o', "output", &output_path,
                   "Output file path "
                   "(default is input file path with replaced wav extension)",
                   NULL, 0, 0),
        OPT_END()};

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);

    argc = argparse_parse(&argparse, argc, argv);

    if (argc < 0)
    {
        argparse_usage(&argparse);
        return -EINVAL;
    }

    /**
     * Input error handling
     */
    if (input_path == NULL)
    {
        print_error("Input path not present\n");
        argparse_usage(&argparse);

        return -EINVAL;
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
        argparse_usage(&argparse);

        return -EINVAL;
    }
    if (frequency <= 0)
    {
        print_error("Frequency has to be a positive number\n");
        argparse_usage(&argparse);

        return -EINVAL;
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
    if (output_path == NULL)
    {
        output_path = fallback_output_path;
        ret = create_output_file_path(input_path, output_path);
    }
    if (ret)
    {
        print_error("Error creating the output path. Please try again.\n");
        return ret;
    }

    params->channels = channels;
    params->chunks = chunks;
    params->interleave = interleave;
    params->offset = offset;
    params->frequency = frequency;
    params->input_file = input_path;
    params->output_file = output_path;
    return ret;
}

int create_output_file_path(char *input_path, char *output_path)
{
    int ret = 0;

#ifdef _WIN32
    char drive[_MAX_DRIVE];
    char directory[_MAX_DIR];
    char filename[_MAX_FNAME];
    ret = _splitpath_s(input_path, drive, _MAX_DRIVE, directory, _MAX_DIR, filename, _MAX_FNAME, NULL, 0);
    if (ret)
    {
        return ret;
    }

    ret = snprintf(output_path, TOTAL_PATH_LEN, "%s%s%s.wav", drive, directory, filename);
    if (ret > TOTAL_PATH_LEN)
    {
        return -EINVAL;
    }
    if (ret < 0)
    {
        return ret;
    }

    return 0;
#elif defined __unix__
    char *directory;
    char *bname;
    char temp_input_path[FILENAME_LEN] = {0}; // Because libgen's basename modifies the original array
    char filename[FILENAME_LEN] = {0};

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
        long filename_length = last_dot - bname;
        uint16_t copy_len = FILENAME_LEN > filename_length ? filename_length : FILENAME_LEN;
        strncpy(filename, bname, copy_len);
        filename[copy_len] = '\0';
    }

    ret = snprintf(output_path, TOTAL_PATH_LEN, "%s/%s.wav", directory, filename);
    if (ret > TOTAL_PATH_LEN)
    {
        return -EINVAL;
    }

    if (ret < 0)
    {
        return ret;
    }

    return 0;
#else
#error Unsupported platform
#endif
}
