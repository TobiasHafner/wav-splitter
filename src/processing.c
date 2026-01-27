#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "processing.h"
#include "utils.h"

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#define MAX_PATH_LENGTH 250


void initialize_session(const char *sessionPath_p, uint64_t *maxChunkIndex, char **outputPath_p) {
    // find highest chunk index
    _find_max_chunk_index(maxChunkIndex, sessionPath_p);
    if (*maxChunkIndex == 0) {
        fprintf(stderr, "ERROR: No input files found\n");
        exit(1);
    }
    
    // create output directory
    *outputPath_p = malloc(strlen(sessionPath_p) + 7);
    if (*outputPath_p == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed\n");
        exit(1);
    }
    sprintf(*outputPath_p, "%s%c%s%c", sessionPath_p, PATH_SEPARATOR, "out", PATH_SEPARATOR);
    _create_output_folder(*outputPath_p);
}


void initialize_buffers(const WavHeader *inputHeader, size_t totalBufferSizeMB, 
                       uint8_t ***writeBuffers_pp, size_t **bufferFillBytes_p, 
                       size_t *bufferSizeBytes) {
    // calculate buffer size per channel
    *bufferSizeBytes = (totalBufferSizeMB * 1024 * 1024) / inputHeader->num_channels;
    printf("Buffer size per channel: %.2f MB\n", *bufferSizeBytes / (1024.0 * 1024.0));

    // allocate buffer arrays
    *writeBuffers_pp = malloc(inputHeader->num_channels * sizeof(uint8_t *));
    *bufferFillBytes_p = malloc(inputHeader->num_channels * sizeof(size_t));
    
    if (*writeBuffers_pp == NULL || *bufferFillBytes_p == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate buffer arrays\n");
        exit(1);
    }

    // allocate individual channel buffers
    for (int i = 0; i < inputHeader->num_channels; i++) {
        (*writeBuffers_pp)[i] = malloc(*bufferSizeBytes);
        (*bufferFillBytes_p)[i] = 0;
        
        if ((*writeBuffers_pp)[i] == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate write buffer for channel %d\n", i + 1);
            exit(1);
        }
    }
}


FILE* read_chunk_header(uint64_t chunkIndex, const char *sessionPath_p, 
                        WavHeader *inputHeader, FILE ***outputFiles_pp, 
                        uint32_t **bytesWritten_p, const char *outputPath_p) {
    // build file path
    char inputFilePath[MAX_PATH_LENGTH];
    sprintf(inputFilePath, "%s%c%08" PRIX64 ".WAV", sessionPath_p, PATH_SEPARATOR, chunkIndex);
    inputFilePath[MAX_PATH_LENGTH - 1] = '\0';

    printf("Processing input file: %s\n", inputFilePath);

    // open file
    FILE *inputFile_p = fopen(inputFilePath, "rb");
    if (!inputFile_p) {
        fprintf(stderr, "ERROR: Failed to open input file\n");
        _cleanup(outputFiles_pp, inputHeader->num_channels, bytesWritten_p);
        exit(-1);
    }

    // read header
    if (read_header(inputFile_p, inputHeader) != 0) {
        fprintf(stderr, "ERROR: Failed to read WAV header\n");
        fclose(inputFile_p);
        _cleanup(outputFiles_pp, inputHeader->num_channels, bytesWritten_p);
        exit(1);
    }

    // initialize output files on first chunk
    if (chunkIndex == 1) {
        _init_output_files(inputFile_p, outputFiles_pp, inputHeader, bytesWritten_p, outputPath_p);
        printf("Created output files for %d channels\n", inputHeader->num_channels);
    }

    return inputFile_p;
}


void extract_audio_from_chunk(FILE *inputFile_p, const WavHeader *inputHeader,
                             uint8_t **writeBuffers_pp, size_t *bufferFillBytes_p,
                             size_t bufferSizeBytes, FILE **outputFiles_pp,
                             uint32_t *bytesWritten_p) {
    // prepare buffer for reading interleaved audio
    const size_t read_buffer_size = inputHeader->block_align;
    uint8_t *read_buffer_p = malloc(read_buffer_size);
    if (!read_buffer_p) {
        fprintf(stderr, "ERROR: Failed to allocate read buffer\n");
        exit(1);
    }

    // extract audio sample by sample
    const uint16_t bytesPerSample = inputHeader->bits_per_sample / 8;
    while (fread(read_buffer_p, read_buffer_size, 1, inputFile_p) == 1) {
        for (int i = 0; i < inputHeader->num_channels; i++) {
            // copy sample to channel buffer
            memcpy(writeBuffers_pp[i] + bufferFillBytes_p[i], 
                   read_buffer_p + i * bytesPerSample, 
                   bytesPerSample);
            bufferFillBytes_p[i] += bytesPerSample;

            // if buffer is full, write to file
            if (bufferFillBytes_p[i] >= bufferSizeBytes) {
                if (fwrite(writeBuffers_pp[i], bufferFillBytes_p[i], 1, outputFiles_pp[i]) != 1) {
                    fprintf(stderr, "ERROR: Writing data to channel %d\n", i + 1);
                    free(read_buffer_p);
                    exit(1);
                }
                bytesWritten_p[i] += bufferFillBytes_p[i];
                bufferFillBytes_p[i] = 0;
            }
        }
    }

    free(read_buffer_p);
}


void flush_remaining_buffers(const WavHeader *inputHeader, uint8_t **writeBuffers_pp,
                            size_t *bufferFillBytes_p, FILE **outputFiles_pp,
                            uint32_t *bytesWritten_p) {
    for (int i = 0; i < inputHeader->num_channels; i++) {
        if (bufferFillBytes_p[i] > 0) {
            if (fwrite(writeBuffers_pp[i], bufferFillBytes_p[i], 1, outputFiles_pp[i]) != 1) {
                fprintf(stderr, "ERROR: Writing remaining data to channel %d\n", i + 1);
                exit(1);
            }
            bytesWritten_p[i] += bufferFillBytes_p[i];
        }
        free(writeBuffers_pp[i]);
    }

    free(writeBuffers_pp);
    free(bufferFillBytes_p);
}


void finalize_output_files(const WavHeader *inputHeader, uint32_t **bytesWritten_p,
                          FILE ***outputFiles_pp) {
    if (*outputFiles_pp) {
        _rewrite_headers(inputHeader, bytesWritten_p, outputFiles_pp);
        _cleanup(outputFiles_pp, inputHeader->num_channels, bytesWritten_p);
    }
    printf("Header rewriting completed and files closed.\n");
}
