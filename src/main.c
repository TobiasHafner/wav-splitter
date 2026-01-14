/*
 * Copyright 2026 Tobias Hafner
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "wav-header.h"
#include "processing-utils.h"

#define MAX_PATH_LENGTH 250

// 4098 MB is enough to store around 7 minutes of 32 channel audio at 24 bit, 96 kHz
#define TOTAL_BUFFER_SIZE_MB 4096

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif


int main(const int argc, char *argv[]) {
    // check for valid input arguments
    if (argc != 2) {
        printf("Usage: wav-splitter <session_path>\n");
        exit(1);
    }

    // get session path and name
    const char *sessionPath_p = argv[1];
    const char *sessionName_p = strdup(argv[1]);
    char *separator_p = strchr(sessionName_p, PATH_SEPARATOR);

    if (separator_p != NULL) {
        *separator_p = '\0';
    }

    // find highest chunk index
    uint64_t maxChunkIndex = 0;
    _find_max_chunk_index(&maxChunkIndex, sessionPath_p);
    if (maxChunkIndex == 0) {
      fprintf(stderr, "ERROR: No input files found\n");
      exit(1);
    }
    
    // create output directory
    char *outputPath_p = malloc(strlen(sessionPath_p) + 7);
    if (outputPath_p == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed\n");
        exit(1);
    }
    sprintf(outputPath_p, "%s%c%s%c", sessionPath_p, PATH_SEPARATOR, "out", PATH_SEPARATOR);
    _create_output_folder(outputPath_p);

    // explode each of the individual chunks into individual channels and append to one file per channel
    WavHeader inputHeader;

    FILE **outputFiles_pp = NULL;
    uint32_t *bytesWritten_p = NULL;

    // prepare buffers
    uint8_t **writeBuffers_pp = NULL;
    size_t *bufferFillBytes_p = NULL;
    size_t bufferSizeBytes = 0;

    for (uint64_t chunkIndex = 1; chunkIndex <= maxChunkIndex; chunkIndex++) {
        // build file path
        char inputFilePath[MAX_PATH_LENGTH];
        sprintf(inputFilePath, "%s%c%08" PRIX64 ".WAV", sessionPath_p, PATH_SEPARATOR, chunkIndex);
        inputFilePath[MAX_PATH_LENGTH - 1] = '\0';

        printf("Processing input file: %s\n", inputFilePath);

        // open file
        FILE *inputFile_p = fopen(inputFilePath, "rb");
        if (!inputFile_p) {
            fprintf(stderr, "ERROR: Failed to open input file\n");
            _cleanup(&outputFiles_pp, inputHeader.num_channels, &bytesWritten_p);
            exit(-1);
        }

        // read header
        if (read_header(inputFile_p, &inputHeader) != 0) {
            fprintf(stderr, "ERROR: Failed to read WAV header\n");
            fclose(inputFile_p);
            _cleanup(&outputFiles_pp, inputHeader.num_channels, &bytesWritten_p);
            exit(1);
        }

        // prepare output files and buffers
        if (chunkIndex == 1) {
            _init_output_files(inputFile_p, &outputFiles_pp, &inputHeader, &bytesWritten_p, outputPath_p);
            printf("Created output files for %d channels\n", inputHeader.num_channels);

            writeBuffers_pp = malloc(inputHeader.num_channels * sizeof(uint8_t *));
            bufferFillBytes_p = malloc(inputHeader.num_channels * sizeof(size_t));
            for (int i = 0; i < inputHeader.num_channels; i++) {
                writeBuffers_pp[i] = malloc(bufferSizeBytes);
                bufferFillBytes_p[i] = 0;
                if (!writeBuffers_pp[i]) {
                    fprintf(stderr, "ERROR: Failed to allocate write buffers\n");
                    fclose(inputFile_p);
                    _cleanup(&outputFiles_pp, inputHeader.num_channels, &bytesWritten_p);
                    exit(1);
                }
            }
        }

        // prepare buffer for audio extraction
        const size_t read_buffer_size = inputHeader.block_align;
        uint8_t *read_buffer_p = malloc(read_buffer_size);
        if (!read_buffer_p) {
            fprintf(stderr, "ERROR: Failed to allocate read buffer\n");
            fclose(inputFile_p);
            _cleanup(&outputFiles_pp, inputHeader.num_channels, &bytesWritten_p);
            exit(1);
        }

        // extract audio
        const uint16_t bytesPerSample = inputHeader.bits_per_sample / 8;
        while (fread(read_buffer_p, read_buffer_size, 1, inputFile_p) == 1) {
            for (int i = 0; i < inputHeader.num_channels; i++) {
                memcpy(writeBuffers_pp[i] + bufferFillBytes_p[i], read_buffer_p + i * bytesPerSample, bytesPerSample);
                bufferFillBytes_p[i] += bytesPerSample;

                // if buffer is full, write to file
                if (bufferFillBytes_p[i] >= bufferSizeBytes) {
                    if (fwrite(writeBuffers_pp[i], bufferFillBytes_p[i], 1, outputFiles_pp[i]) != 1) {
                        fprintf(stderr, "Error writing data to channel %d\n", i + 1);
                        _cleanup(&outputFiles_pp, inputHeader.num_channels, &bytesWritten_p);
                        exit(1);
                    }
                    bytesWritten_p[i] += bufferFillBytes_p[i];
                    bufferFillBytes_p[i] = 0;
                }
            }
        }

        free(read_buffer_p);
        fclose(inputFile_p);
    }

    // write remaining data from buffers to file
    for (int i = 0; i < inputHeader.num_channels; i++) {
        if (bufferFillBytes_p[i] > 0) {
            if (fwrite(writeBuffers_pp[i], bufferFillBytes_p[i], 1, outputFiles_pp[i]) != 1) {
                fprintf(stderr, "Error writing remaining data to channel %d\n", i + 1);
                _cleanup(&outputFiles_pp, inputHeader.num_channels, &bytesWritten_p);
                exit(1);
            }
            bytesWritten_p[i] += bufferFillBytes_p[i];
        }
        free(writeBuffers_pp[i]);
    }

    free(writeBuffers_pp);
    free(bufferFillBytes_p);

    // update headers with correct file sizes after all data has been written
    if (outputFiles_pp) {
        _rewrite_headers(&inputHeader, &bytesWritten_p, &outputFiles_pp);
        _cleanup(&outputFiles_pp, inputHeader.num_channels, &bytesWritten_p);
    }

    printf("Header rewriting completed and files closed.\n");

    return 0;
}
