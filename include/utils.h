/**
 * @file utils.h
 * @brief Utility functions for processing multi-channel WAV files
 *
 * This header file contains the definition of a collection of low-level utility
 * functions used to process multi-channel WAV files, including file I/O operations,
 * directory management, and WAV header manipulation.
 *
 * @author Tobias Hafner
 * @date 2026-01-27
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include "wav-header.h"

/**
 * Clean up and close all output files and free allocated memory
 *
 * Closes all open file handles in the output files array and frees
 * the memory allocated for the files array and bytes written tracking array.
 *
 * @param outputFiles Pointer to array of output file handles to close and free
 * @param count Number of channels (output files) to clean up
 * @param bytesWritten Pointer to array tracking bytes written per channel to free
 */
void _cleanup(FILE ***outputFiles, uint16_t count, uint32_t **bytesWritten);

/**
 * Create the output folder for processed WAV files
 *
 * Creates a directory at the specified path with appropriate permissions.
 * Platform-specific implementation for Windows and Unix-like systems.
 * Exits with error if creation fails (unless directory already exists on Windows).
 *
 * @param outputPath Path to the output directory to create
 */
void _create_output_folder(char *outputPath);

/**
 * Find the highest chunk index in the session directory
 *
 * Scans the session directory for WAV files with hexadecimal naming (e.g., 00000001.WAV)
 * and determines the maximum chunk index. This is used to know how many chunks to process.
 *
 * @param maxChunkIndex Pointer to store the maximum chunk index found (0 if no files found)
 * @param sessionPath Path to the session directory to scan
 */
void _find_max_chunk_index(uint64_t *maxChunkIndex, const char *sessionPath);

/**
 * Initialize output files for each channel
 *
 * Creates one output WAV file per channel, writes initial headers (with placeholder sizes),
 * and initializes tracking arrays for file handles and bytes written. The headers will be
 * rewritten with correct sizes after all data has been written.
 *
 * @param inputFile Input file handle (used for error handling context)
 * @param outputFiles Pointer to array of output file handles (allocated by this function)
 * @param inputHeader WAV header from input file containing format information
 * @param dataWritten Pointer to array tracking bytes written per channel (allocated by this function)
 * @param outputPath Path to the output directory where channel files will be created
 */
void _init_output_files(FILE *inputFile, FILE ***outputFiles, const WavHeader *inputHeader, 
                        uint32_t **dataWritten, const char *outputPath);

/**
 * Rewrite WAV headers with correct file sizes
 *
 * After all audio data has been written, this function seeks back to the beginning
 * of each output file and rewrites the WAV header with the correct data_bytes and
 * wav_size fields based on the actual amount of data written.
 *
 * @param inputHeader Original input WAV header containing format information
 * @param dataWritten Pointer to array containing actual bytes written per channel
 * @param outputFiles Pointer to array of output file handles to update
 */
void _rewrite_headers(const WavHeader *inputHeader, uint32_t **dataWritten, FILE ***outputFiles);

#endif // PROCESSING_UTILS_H
