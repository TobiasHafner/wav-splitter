#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "wav-splitter.h"

int read_chunk_id(FILE *inputFile, char *chunkId) {
    if (fread(chunkId, 4, 1, inputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to read chunk ID\n");
        return -1;
    }
    return 0;
}

int read_chunk_size(FILE *inputFile, uint32_t *chunkSize) {
    if (fread(chunkSize, sizeof(*chunkSize), 1, inputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to read chunk size\n");
        return -1;
    }
    return 0;
}

int read_header(FILE *inputFile, struct WavHeader *header) {
    char chunkId[4];
    uint32_t chunkSize = 0;

    // Read initial "RIFF" and its size
    if (fread(header->riff_header, sizeof(header->riff_header), 1, inputFile) != 1 ||
        fread(&header->wav_size, sizeof(header->wav_size), 1, inputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to read initial RIFF header\n");
        return -1;
    }

    // Read format "WAVE"
    if (fread(header->wave_header, sizeof(header->wave_header), 1, inputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to read WAVE header\n");
        return -1;
    }

    // Find 'fmt ' chunk
    while (1) {
        if (fread(chunkId, sizeof(chunkId), 1, inputFile) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk ID\n");
            return -1;
        }
        if (fread(&chunkSize, sizeof(chunkSize), 1, inputFile) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk size\n");
            return -1;
        }
        if (strncmp(chunkId, "fmt ", 4) == 0) {
            memcpy(header->fmt_header, chunkId, sizeof(chunkId)); // Correct assignment of chunkId to fmt_header
            header->fmt_chunk_size = chunkSize;
            if (chunkSize != 16) {
                fprintf(stderr, "ERROR: Unexpected fmt chunk size\n");
                return -1;
            }
            // Read the fmt content correctly
            if (fread(&header->audio_format, sizeof(uint16_t), 1, inputFile) != 1 ||
                fread(&header->num_channels, sizeof(uint16_t), 1, inputFile) != 1 ||
                fread(&header->sample_rate, sizeof(uint32_t), 1, inputFile) != 1 ||
                fread(&header->byte_rate, sizeof(uint32_t), 1, inputFile) != 1 ||
                fread(&header->block_align, sizeof(uint16_t), 1, inputFile) != 1 ||
                fread(&header->bits_per_sample, sizeof(uint16_t), 1, inputFile) != 1) {
                fprintf(stderr, "ERROR: Failed to read fmt sub-chunk\n");
                return -1;
            }
            break; // fmt chunk found and read, exit loop
        } else {
            // Skip chunk if not 'fmt '
            fseek(inputFile, chunkSize, SEEK_CUR);
        }
    }

    // Find 'data' chunk
    while (1) {
        if (fread(chunkId, sizeof(chunkId), 1, inputFile) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk ID\n");
            return -1;
        }
        if (fread(&chunkSize, sizeof(chunkSize), 1, inputFile) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk size\n");
            return -1;
        }
        if (strncmp(chunkId, "data", 4) == 0) {
            memcpy(header->data_header, chunkId, sizeof(chunkId)); // Correct assignment of chunkId to data_header
            header->data_bytes = chunkSize;
            break; // 'data' chunk found, no need to read its data here
        } else {
            // Skip other chunks
            fseek(inputFile, chunkSize, SEEK_CUR);
        }
    }
    return 0;
}

int write_header(FILE *outputFile, const struct WavHeader *inputHeader) {
    struct WavHeader outHeader = *inputHeader;
    outHeader.num_channels = 1;
    outHeader.byte_rate = outHeader.sample_rate * outHeader.bits_per_sample / 8;
    outHeader.block_align = outHeader.bits_per_sample / 8;
    outHeader.data_bytes = inputHeader->data_bytes / inputHeader->num_channels;

    // Adjust the size of the 'RIFF' chunk, which is the entire file size minus 8 bytes for 'RIFF' and 'wav_size'
    outHeader.wav_size = 36 + outHeader.data_bytes; // 36 = 4 ('WAVE') + 24 ('fmt ' chunk) + 8 ('data' header and size)

    // Writing the 'RIFF' chunk descriptor
    if (fwrite(&outHeader.riff_header, sizeof(outHeader.riff_header), 1, outputFile) != 1 ||
        fwrite(&outHeader.wav_size, sizeof(outHeader.wav_size), 1, outputFile) != 1 ||
        fwrite(&outHeader.wave_header, sizeof(outHeader.wave_header), 1, outputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to write RIFF chunk to output file\n");
        return -1;
    }

    // Writing the 'fmt ' sub-chunk
    if (fwrite(&outHeader.fmt_header, sizeof(outHeader.fmt_header), 1, outputFile) != 1 ||
        fwrite(&outHeader.fmt_chunk_size, sizeof(outHeader.fmt_chunk_size), 1, outputFile) != 1 ||
        fwrite(&outHeader.audio_format, sizeof(outHeader.audio_format), 1, outputFile) != 1 ||
        fwrite(&outHeader.num_channels, sizeof(outHeader.num_channels), 1, outputFile) != 1 ||
        fwrite(&outHeader.sample_rate, sizeof(outHeader.sample_rate), 1, outputFile) != 1 ||
        fwrite(&outHeader.byte_rate, sizeof(outHeader.byte_rate), 1, outputFile) != 1 ||
        fwrite(&outHeader.block_align, sizeof(outHeader.block_align), 1, outputFile) != 1 ||
        fwrite(&outHeader.bits_per_sample, sizeof(outHeader.bits_per_sample), 1, outputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to write 'fmt ' chunk to output file\n");
        return -1;
    }

    // Writing the 'data' sub-chunk header
    if (fwrite(&outHeader.data_header, sizeof(outHeader.data_header), 1, outputFile) != 1 ||
        fwrite(&outHeader.data_bytes, sizeof(outHeader.data_bytes), 1, outputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to write data chunk header to output file\n");
        return -1;
    }

    return 0;
}


int create_output_files(const struct WavHeader *inputHeader, const char *inputFileName, FILE ***outputFiles) {
    *outputFiles = malloc(inputHeader->num_channels * sizeof(FILE *));
    if (!*outputFiles) return -1;

    for (int i = 0; i < inputHeader->num_channels; i++) {
        char outputFileName[100];
        snprintf(outputFileName, sizeof(outputFileName), "%s_channel_%d.wav", inputFileName, i + 1);
        (*outputFiles)[i] = fopen(outputFileName, "wb");

        // on fail: free all previously created files
        if (!(*outputFiles)[i]) {
            for (int j = 0; j < i; j++) {
                fclose((*outputFiles)[j]);
            }
            free(*outputFiles);
            return -1;
        }

        // write header to each new file
        if (write_header((*outputFiles)[i], inputHeader) == -1) {
            for (int j = 0; j <= i; j++) fclose((*outputFiles)[j]);
            free(outputFiles);
            return -1;
        }
    }
    return 0;
}

int split_wav_file(FILE *inputFile, const struct WavHeader *inputHeader, const char *inputFileName) {
    FILE **outputFiles;
    if (create_output_files(inputHeader, inputFileName, &outputFiles) != 0) {
        fprintf(stderr, "Error creating output files\n");
        return -1;
    }

    const size_t buffer_size = inputHeader->block_align;

    uint8_t *buffer = malloc(buffer_size);
    if (!buffer) {
        for (int i = 0; i < inputHeader->num_channels; i++) {
            if (outputFiles[i]) fclose(outputFiles[i]);
        }
        fprintf(stderr, "Failed to allocate output buffer\n");
        free(outputFiles);
        return -1;
    }

    while (fread(buffer, buffer_size, 1, inputFile) == 1) {
        for (int i = 0; i < inputHeader->num_channels; i++) {
            const int bytesPerSample = 4;
            if (fwrite(buffer + i * bytesPerSample, bytesPerSample, 1, outputFiles[i]) != 1) {
                fprintf(stderr, "Error writing data to channel %d\n", i + 1);
            }
        }
    }

    // Clean up
    for (int i = 0; i < inputHeader->num_channels; i++) {
        if (outputFiles[i]) fclose(outputFiles[i]);
    }
    free(outputFiles);
    free(buffer);

    return 0;
}
