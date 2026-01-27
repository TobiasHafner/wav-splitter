#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "wav-header.h"
#include "processing.h"

// Default buffer size: 4096 MB is enough to store around 7 minutes of 32 channel audio at 24 bit, 96 kHz
#define DEFAULT_BUFFER_SIZE_MB 4096


/**
 * Parse command line arguments
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @param sessionPath_p Pointer to store the session path
 * @param totalBufferSizeMB Pointer to store the buffer size in MB
 */
static void parse_arguments(int argc, char *argv[], const char **sessionPath_p, size_t *totalBufferSizeMB) {
    *totalBufferSizeMB = DEFAULT_BUFFER_SIZE_MB;
    
    // check for valid input arguments
    if (argc < 2 || argc > 4) {
        printf("Usage: wav-splitter [-m buffer_size_mb] <session_path>\n");
        printf("  -m buffer_size_mb : Optional total buffer size in MB (default: %d)\n", DEFAULT_BUFFER_SIZE_MB);
        exit(1);
    }
    
    // check for -m flag
    int argIndex = 1;
    if (argc >= 3 && strcmp(argv[argIndex], "-m") == 0) {
        char *endptr;
        long bufferSize = strtol(argv[argIndex + 1], &endptr, 10);
        
        if (*endptr != '\0' || bufferSize <= 0) {
            fprintf(stderr, "ERROR: Invalid buffer size '%s'\n", argv[argIndex + 1]);
            exit(1);
        }
        
        *totalBufferSizeMB = (size_t)bufferSize;
        argIndex += 2;
        printf("Using buffer size: %zu MB\n", *totalBufferSizeMB);
    }
    
    // get session path
    if (argIndex >= argc) {
        fprintf(stderr, "ERROR: Session path not provided\n");
        exit(1);
    }
    *sessionPath_p = argv[argIndex];
}


int main(const int argc, char *argv[]) {
    // parse command line arguments
    const char *sessionPath_p = NULL;
    size_t totalBufferSizeMB = 0;
    parse_arguments(argc, argv, &sessionPath_p, &totalBufferSizeMB);

    // initialize session and find chunks
    uint64_t maxChunkIndex = 0;
    char *outputPath_p = NULL;
    initialize_session(sessionPath_p, &maxChunkIndex, &outputPath_p);

    // prepare processing state
    WavHeader inputHeader;
    FILE **outputFiles_pp = NULL;
    uint32_t *bytesWritten_p = NULL;
    uint8_t **writeBuffers_pp = NULL;
    size_t *bufferFillBytes_p = NULL;
    size_t bufferSizeBytes = 0;

    for (uint64_t chunkIndex = 1; chunkIndex <= maxChunkIndex; chunkIndex++) {
        // read chunk header and initialize output files on first chunk
        FILE *inputFile_p = read_chunk_header(chunkIndex, sessionPath_p, &inputHeader, 
                                             &outputFiles_pp, &bytesWritten_p, outputPath_p);
                                             
        if (chunkIndex == 1) {
            initialize_buffers(&inputHeader, totalBufferSizeMB, &writeBuffers_pp, 
                             &bufferFillBytes_p, &bufferSizeBytes);
        }

        extract_audio_from_chunk(inputFile_p, &inputHeader, writeBuffers_pp, bufferFillBytes_p,
                                bufferSizeBytes, outputFiles_pp, bytesWritten_p);

        fclose(inputFile_p);
    }

    flush_remaining_buffers(&inputHeader, writeBuffers_pp, bufferFillBytes_p, 
                           outputFiles_pp, bytesWritten_p);

    finalize_output_files(&inputHeader, &bytesWritten_p, &outputFiles_pp);

    free(outputPath_p);
    return 0;
}
