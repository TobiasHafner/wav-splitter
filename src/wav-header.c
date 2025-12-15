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

int read_header(FILE *inputFile_p, WavHeader *header_p) {
    char currentChunkName[4];
    uint32_t currentChunkSize;

    // read RIFF header
    if (fread(header_p->riff_header, sizeof(header_p->riff_header), 1, inputFile_p) != 1 ||
        fread(&header_p->wav_size, sizeof(header_p->wav_size), 1, inputFile_p) != 1) {
        fprintf(stderr, "ERROR: Failed to read initial RIFF header\n");
        return -1;
    }

    // read WAVE header
    if (fread(header_p->wave_header, sizeof(header_p->wave_header), 1, inputFile_p) != 1) {
        fprintf(stderr, "ERROR: Failed to read WAVE header\n");
        return -1;
    }

    // find the fmt chunk by skipping over other ones
    while (1) {
        // read current chunk id
        if (fread(currentChunkName, sizeof(currentChunkName), 1, inputFile_p) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk ID\n");
            return -1;
        }

        // read the size of the current chunk
        if (fread(&currentChunkSize, sizeof(currentChunkSize), 1, inputFile_p) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk size\n");
            return -1;
        }

        // check if the current chunk is the fmt chunk
        if (strncmp(currentChunkName, "fmt ", 4) == 0) {
            memcpy(header_p->fmt_header, currentChunkName, sizeof(currentChunkName));
            header_p->fmt_chunk_size = currentChunkSize;
            break;
        }

        // skip chunk
        if (fseek(inputFile_p, currentChunkSize, SEEK_CUR) != 0) {
            fprintf(stderr, "ERROR: Failed to skip chunk\n");
            return -1;
        }
    }

    // read the fmt chunk
    if (fread(&header_p->audio_format, sizeof(header_p->audio_format), 1, inputFile_p) != 1 ||
        fread(&header_p->num_channels, sizeof(header_p->num_channels), 1, inputFile_p) != 1 ||
        fread(&header_p->sample_rate, sizeof(header_p->sample_rate), 1, inputFile_p) != 1 ||
        fread(&header_p->byte_rate, sizeof(header_p->byte_rate), 1, inputFile_p) != 1 ||
        fread(&header_p->block_align, sizeof(header_p->block_align), 1, inputFile_p) != 1 ||
        fread(&header_p->bits_per_sample, sizeof(header_p->bits_per_sample), 1, inputFile_p) != 1) {
        fprintf(stderr, "ERROR: Failed to read fmt-chunk\n");
        return -1;
    }

    // find the data chunk by skipping over other ones
    while (1) {
        // read current chunk id
        if (fread(currentChunkName, sizeof(currentChunkName), 1, inputFile_p) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk ID\n");
            return -1;
        }

        // read current chunk size
        if (fread(&currentChunkSize, sizeof(currentChunkSize), 1, inputFile_p) != 1) {
            fprintf(stderr, "ERROR: Failed to read chunk size\n");
            return -1;
        }

        // check if the current chunk is the data chunk
        if (strncmp(currentChunkName, "data", 4) == 0) {
            memcpy(header_p->data_header, currentChunkName, sizeof(currentChunkName));
            header_p->data_bytes = currentChunkSize;
            break;
        }

        // skip chunk
        if (fseek(inputFile_p, currentChunkSize, SEEK_CUR) != 0) {
            fprintf(stderr, "ERROR: Failed to skip chunk\n");
            return -1;
        }
    }

    return 0;
}

int write_header(FILE *outputFile_p, const WavHeader *header_p) {
    // write RIFF header
    if (fwrite(header_p->riff_header, sizeof(header_p->riff_header), 1, outputFile_p) != 1 ||
        fwrite(&header_p->wav_size, sizeof(header_p->wav_size), 1, outputFile_p) != 1) {
        fprintf(stderr, "ERROR: Failed to write RIFF chunk to output file\n");
        return -1;
        }

    // write WAVE header
    if (fwrite(header_p->wave_header, sizeof(header_p->wave_header), 1, outputFile_p) != 1) {
        fprintf(stderr, "ERROR: Failed to write WAVE header to output file\n");
        return -1;
    }

    // write fmt chunk
    if (fwrite(header_p->fmt_header, sizeof(header_p->fmt_header), 1, outputFile_p) != 1 ||
        fwrite(&header_p->fmt_chunk_size, sizeof(header_p->fmt_chunk_size), 1, outputFile_p) != 1 ||
        fwrite(&header_p->audio_format, sizeof(header_p->audio_format), 1, outputFile_p) != 1 ||
        fwrite(&header_p->num_channels, sizeof(header_p->num_channels), 1, outputFile_p) != 1 ||
        fwrite(&header_p->sample_rate, sizeof(header_p->sample_rate), 1, outputFile_p) != 1 ||
        fwrite(&header_p->byte_rate, sizeof(header_p->byte_rate), 1, outputFile_p) != 1 ||
        fwrite(&header_p->block_align, sizeof(header_p->block_align), 1, outputFile_p) != 1 ||
        fwrite(&header_p->bits_per_sample, sizeof(header_p->bits_per_sample), 1, outputFile_p) != 1) {
        fprintf(stderr, "ERROR: Failed to write 'fmt ' chunk to output file\n");
        return -1;
        }

    // write data chunk header
    if (fwrite(header_p->data_header, sizeof(header_p->data_header), 1, outputFile_p) != 1 ||
        fwrite(&header_p->data_bytes, sizeof(header_p->data_bytes), 1, outputFile_p) != 1) {
        fprintf(stderr, "ERROR: Failed to write data chunk header to output file\n");
        return -1;
        }

    return 0;
}


int create_output_files(const WavHeader *inputHeader_p, const char *basePath_p, FILE ***outputFiles_ppp) {
    *outputFiles_ppp = malloc(inputHeader_p->num_channels * sizeof(FILE *));
    if (!*outputFiles_ppp) {
        return -1;
    }

    for (int i = 0; i < inputHeader_p->num_channels; i++) {
        char outputFileName[260];
        snprintf(outputFileName, sizeof(outputFileName), "%sch_%d.wav", basePath_p, i + 1);

        (*outputFiles_ppp)[i] = fopen(outputFileName, "wb");

        if (!(*outputFiles_ppp)[i]) {
            for (int j = 0; j < i; j++) {
                fclose((*outputFiles_ppp)[j]);
            }
            free(*outputFiles_ppp);
            return -1;
        }

        if (write_header((*outputFiles_ppp)[i], inputHeader_p) == -1) {
            for (int j = 0; j <= i; j++) fclose((*outputFiles_ppp)[j]);
            free(outputFiles_ppp);
            return -1;
        }
    }
    return 0;
}
