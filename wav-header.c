#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "wav-header.h"

#ifdef WIN32
#include <windows.h>
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

int read_header(FILE *inputFile, WavHeader *header) {
    char currentChunkName[4];
    uint32_t currentChunkSize;

    // read RIFF header
    if (fread(header->riff_header, sizeof(header->riff_header), 1, inputFile) != 1 ||
        fread(&header->wav_size, sizeof(header->wav_size), 1, inputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to read initial RIFF header\n");
        return -1;
    }

    // read WAVE header
    if (fread(header->wave_header, sizeof(header->wave_header), 1, inputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to read WAVE header\n");
        return -1;
    }

    // find the fmt chunk by skipping over other ones
    while (1) {
        // read current chunk id
        if (fread(currentChunkName, sizeof(currentChunkName), 1, inputFile) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk ID\n");
            return -1;
        }

        // read the size of the current chunk
        if (fread(&currentChunkSize, sizeof(currentChunkSize), 1, inputFile) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk size\n");
            return -1;
        }

        // check if the current chunk is the fmt chunk
        if (strncmp(currentChunkName, "fmt ", 4) == 0) {
            memcpy(header->fmt_header, currentChunkName, sizeof(currentChunkName));
            header->fmt_chunk_size = currentChunkSize;
            break;
        }

        // skip chunk
        if (fseek(inputFile, currentChunkSize, SEEK_CUR) != 0) {
            fprintf(stderr, "ERROR: Failed to skip chunk\n");
            return -1;
        }
    }

    // read the fmt chunk
    if (fread(&header->audio_format, sizeof(header->audio_format), 1, inputFile) != 1 ||
        fread(&header->num_channels, sizeof(header->num_channels), 1, inputFile) != 1 ||
        fread(&header->sample_rate, sizeof(header->sample_rate), 1, inputFile) != 1 ||
        fread(&header->byte_rate, sizeof(header->byte_rate), 1, inputFile) != 1 ||
        fread(&header->block_align, sizeof(header->block_align), 1, inputFile) != 1 ||
        fread(&header->bits_per_sample, sizeof(header->bits_per_sample), 1, inputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to read fmt-chunk\n");
        return -1;
    }

    // find the data chunk by skipping over other ones
    while (1) {
        // read current chunk id
        if (fread(currentChunkName, sizeof(currentChunkName), 1, inputFile) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk ID\n");
            return -1;
        }

        // read current chunk size
        if (fread(&currentChunkSize, sizeof(currentChunkSize), 1, inputFile) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk size\n");
            return -1;
        }

        // check if the current chunk is the data chunk
        if (strncmp(currentChunkName, "data", 4) == 0) {
            memcpy(header->data_header, currentChunkName, sizeof(currentChunkName));
            header->data_bytes = currentChunkSize;
            break;
        }

        // skip chunk
        if (fseek(inputFile, currentChunkSize, SEEK_CUR) != 0) {
            fprintf(stderr, "ERROR: Failed to skip chunk\n");
            return -1;
        }
    }

    return 0;
}

int write_header(FILE *outputFile, const WavHeader *header) {
    // write RIFF header
    if (fwrite(header->riff_header, sizeof(header->riff_header), 1, outputFile) != 1 ||
        fwrite(&header->wav_size, sizeof(header->wav_size), 1, outputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to write RIFF chunk to output file\n");
        return -1;
        }

    // write WAVE header
    if (fwrite(header->wave_header, sizeof(header->wave_header), 1, outputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to write WAVE header to output file\n");
        return -1;
    }

    // write fmt chunk
    if (fwrite(header->fmt_header, sizeof(header->fmt_header), 1, outputFile) != 1 ||
        fwrite(&header->fmt_chunk_size, sizeof(header->fmt_chunk_size), 1, outputFile) != 1 ||
        fwrite(&header->audio_format, sizeof(header->audio_format), 1, outputFile) != 1 ||
        fwrite(&header->num_channels, sizeof(header->num_channels), 1, outputFile) != 1 ||
        fwrite(&header->sample_rate, sizeof(header->sample_rate), 1, outputFile) != 1 ||
        fwrite(&header->byte_rate, sizeof(header->byte_rate), 1, outputFile) != 1 ||
        fwrite(&header->block_align, sizeof(header->block_align), 1, outputFile) != 1 ||
        fwrite(&header->bits_per_sample, sizeof(header->bits_per_sample), 1, outputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to write 'fmt ' chunk to output file\n");
        return -1;
        }

    // write data chunk header
    if (fwrite(header->data_header, sizeof(header->data_header), 1, outputFile) != 1 ||
        fwrite(&header->data_bytes, sizeof(header->data_bytes), 1, outputFile) != 1) {
        fprintf(stderr, "ERROR: Failed to write data chunk header to output file\n");
        return -1;
        }

    return 0;
}


int create_output_files(const WavHeader *inputHeader, const char *basePath, FILE ***outputFiles) {
    *outputFiles = malloc(inputHeader->num_channels * sizeof(FILE *));
    if (!*outputFiles) {
        return -1;
    }

    for (int i = 0; i < inputHeader->num_channels; i++) {
        char outputFileName[260];
        snprintf(outputFileName, sizeof(outputFileName), "%sch_%d.wav", basePath, i + 1);

        (*outputFiles)[i] = fopen(outputFileName, "wb");

        if (!(*outputFiles)[i]) {
            for (int j = 0; j < i; j++) {
                fclose((*outputFiles)[j]);
            }
            free(*outputFiles);
            return -1;
        }

        if (write_header((*outputFiles)[i], inputHeader) == -1) {
            for (int j = 0; j <= i; j++) fclose((*outputFiles)[j]);
            free(outputFiles);
            return -1;
        }
    }
    return 0;
}
