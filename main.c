#include <stdlib.h>
#include <string.h>

#include "wav-header.h"
#include "processing-utils.h"

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif


int main(const int argc, char *argv[]) {
    // check for valid input arguments
    if (argc != 2) {
        printf("Usage: %s <session_path>");
        exit(1);
    }

    // get session path and name
    const char *sessionPath = argv[1];
    const char *sessionName = strdup(argv[1]);
    char *separator = strchr(sessionName, PATH_SEPARATOR);

    if (separator != NULL) {
        *separator = '\0';
    }

    // create ouput folder
    char *outputPath = malloc(strlen(sessionPath) + 7);
    if (outputPath == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed\n");
        exit(1);
    }
    sprintf(outputPath, "%s%c%s%c", sessionPath, PATH_SEPARATOR, "out", PATH_SEPARATOR);
    _create_output_folder(outputPath);

    // find highest chunk index
    uint64_t maxChunkIndex = 0;
    _find_max_chunk_index(&maxChunkIndex, sessionPath);

    // explode each of the individual chunks into individual channels adn append to one file per channel
    WavHeader inputHeader;

    FILE **outputFiles = NULL;
    uint32_t *bytesWritten = NULL;

    for (uint64_t chunkIndex = 1; chunkIndex <= maxChunkIndex; chunkIndex++) {
        // build file path
        char inputFilePath[260];
        sprintf(inputFilePath, "%s%c%08llX.WAV", sessionPath, PATH_SEPARATOR, chunkIndex);

        printf("Processing input file: %s\n", inputFilePath);

        // open file
        FILE *inputFile = fopen(inputFilePath, "rb");
        if (!inputFile) {
            fprintf(stderr, "ERROR: Failed to open input file\n");
            _cleanup(&outputFiles, inputHeader.num_channels, &bytesWritten);
            exit(-1);
        }

        // read header
        if (read_header(inputFile, &inputHeader) != 0) {
            fprintf(stderr, "ERROR: Failed to read WAV header\n");
            fclose(inputFile);
            _cleanup(&outputFiles, inputHeader.num_channels, &bytesWritten);
            exit(1);
        }

        // prepare output files
        if (chunkIndex == 1) {
            _init_output_files(inputFile, &outputFiles, &inputHeader, &bytesWritten, outputPath);
            printf("Created output files for %d channels.\n", inputHeader.num_channels);
        }


        // prepare buffer for audio extraction
        const size_t audio_buffer_size = inputHeader.block_align;
        uint8_t *audio_buffer = malloc(audio_buffer_size);
        if (!audio_buffer) {
            fprintf(stderr, "ERROR: Failed to allocate output buffer\n");
            fclose(inputFile);
            _cleanup(&outputFiles, inputHeader.num_channels, &bytesWritten);
            exit(1);
        }

        // extract audio
        const uint16_t bytesPerSample = inputHeader.bits_per_sample / 8;
        while (fread(audio_buffer, audio_buffer_size, 1, inputFile) == 1) {
            for (int i = 0; i < inputHeader.num_channels; i++) {
                if (fwrite(audio_buffer + i * bytesPerSample, bytesPerSample, 1, outputFiles[i]) != 1) {
                    fprintf(stderr, "Error writing data to channel %d\n", i + 1);
                    _cleanup(&outputFiles, inputHeader.num_channels, &bytesWritten);
                    exit(1);
                }
                bytesWritten[i] += bytesPerSample;
            }
        }

        free(audio_buffer);

        fclose(inputFile);
    }

    // update headers with correct file sizes after all data has been written
    if (outputFiles) {
        _rewrite_headers(&inputHeader, &bytesWritten, &outputFiles);
        _cleanup(&outputFiles, inputHeader.num_channels, &bytesWritten);
    }

    printf("Header rewriting completed and files closed.\n");

    return 0;
}
