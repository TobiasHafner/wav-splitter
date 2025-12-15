/**
* @file processing-utils.h
 * @brief Utils for processing wav files
 *
 * This header file contains the definition of a collectio of functions used to process multichannel wav files.
 *
 * @author Tobias Hafner
 * @date 2024-05-07
 */

#ifndef PROCESSING_UTILS_H
#define PROCESSING_UTILS_H

#include <stdio.h>
#include <stdint.h>

#include "wav-header.h"

void _cleanup(FILE ***outputFiles, uint16_t count, uint32_t **bytesWritten);
void _create_output_folder(char *outputPath);
void _find_max_chunk_index(uint64_t *maxChunkIndex, const char *sessionPath);
void _init_output_files(FILE *inputFile, FILE ***outputFiles, const WavHeader *inputHeader, uint32_t **dataWritten,
                  char *outputPath);
void _rewrite_headers(const WavHeader *inputHeader, uint32_t **dataWritten, FILE ***outputFiles);

#endif
