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

int read_header(FILE *inputFile_p, WavHeader *header_p);

int write_header(FILE *outputFile_p, const WavHeader *header_p);

int create_output_files(const WavHeader *inputHeader_p, const char* basePath_p, FILE ***outputFiles_ppp);

int split_wav_file(FILE *inputFile_p, const WavHeader *inputHeader_p, const char *inputFileName_p);
#endif
