/**
 * @file wav-header.h
 * @brief WAV File Splitter
 *
 * This header file contains the definition of a struct used to store the header data of a wav file.
 *
 * @author Tobias Hafner
 * @date 2024-05-07
 */

#ifndef WAV_HEADER_H
#define WAV_HEADER_H

#include <stdint.h>
#include <stdio.h>

typedef struct {
    char riff_header[4];      // "RIFF" identifier
    uint32_t wav_size;        // Size of the WAV portion minus 8 bytes
    char wave_header[4];      // "WAVE" identifier
    char fmt_header[4];       // "fmt " sub-chunk identifier (with space)
    uint32_t fmt_chunk_size;  // Length of format data as listed above
    uint16_t audio_format;    // Audio format (1 for PCM)
    uint16_t num_channels;    // Number of channels
    uint32_t sample_rate;     // Sampling rate
    uint32_t byte_rate;       // Bytes per second
    uint16_t block_align;     // Bytes per sample block (all channels)
    uint16_t bits_per_sample; // Bits per sample
    char data_header[4];      // "data" sub-chunk identifier
    uint32_t data_bytes;      // Number of bytes in data
} WavHeader;

int read_header(FILE *inputFile, WavHeader *header);

int write_header(FILE *outputFile, const WavHeader *header);

// splitting operations
int create_output_files(const WavHeader *inputHeader, const char* basePath, FILE ***outputFiles);

int split_wav_file(FILE *inputFile, const WavHeader *inputHeader, const char *inputFileName);
#endif
