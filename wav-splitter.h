/**
 * @file wavsplitter.h
 * @brief WAV File Splitter
 *
 * This header file contains definitions and function prototypes for splitting
 * multi-channel WAV files into separate mono-channel WAV files.
 *
 * @author Tobias Hafner
 * @date 2024-05-06
 */

#ifndef WAV_SPLITTER_H
#define WAV_SPLITTER_H

#include <stdint.h>
#include <stdio.h>

typedef struct WavHeader {
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
} wav_header;

int read_chunk_id(FILE *inputFile, char *chunkId);

int read_chunk_size(FILE *inputFile, uint32_t *chunkSize);

int read_header(FILE *inputFile, struct WavHeader *header);

int write_header(FILE *outputFile, const struct WavHeader *inputHeader);

int create_output_files(const struct WavHeader* inputHeader, const char* inputFileName, FILE*** outputFiles);

int split_wav_file(FILE *inputFile, const struct WavHeader* inputHeader, const char *inputFileName);

#endif
