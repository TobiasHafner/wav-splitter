#ifndef PROCESSING_H
#define PROCESSING_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "wav-header.h"

/**
 * Initialize the session by finding the maximum chunk index and creating output directory
 * 
 * @param sessionPath_p Path to the session directory
 * @param maxChunkIndex Pointer to store the maximum chunk index found
 * @param outputPath_p Pointer to store the allocated output path string
 */
void initialize_session(const char *sessionPath_p, uint64_t *maxChunkIndex, char **outputPath_p);

/**
 * Initialize write buffers for all channels
 * 
 * @param inputHeader WAV header containing channel information
 * @param totalBufferSizeMB Total buffer size in megabytes to allocate across all channels
 * @param writeBuffers_pp Pointer to array of write buffers (one per channel)
 * @param bufferFillBytes_p Pointer to array tracking bytes filled in each buffer
 * @param bufferSizeBytes Pointer to store the calculated buffer size per channel in bytes
 */
void initialize_buffers(const WavHeader *inputHeader, size_t totalBufferSizeMB, 
                       uint8_t ***writeBuffers_pp, size_t **bufferFillBytes_p, 
                       size_t *bufferSizeBytes);

/**
 * Read chunk header and initialize output files on first chunk
 * 
 * @param chunkIndex Current chunk index being processed
 * @param sessionPath_p Path to the session directory
 * @param inputHeader Pointer to WAV header structure to populate
 * @param outputFiles_pp Pointer to array of output file handles
 * @param bytesWritten_p Pointer to array tracking bytes written per channel
 * @param outputPath_p Path to output directory
 * @return Opened input file handle (caller must close after processing)
 */
FILE* read_chunk_header(uint64_t chunkIndex, const char *sessionPath_p, 
                        WavHeader *inputHeader, FILE ***outputFiles_pp, 
                        uint32_t **bytesWritten_p, const char *outputPath_p);

/**
 * Extract audio data from current chunk and distribute to channel buffers
 * 
 * @param inputFile_p Input WAV file handle
 * @param inputHeader WAV header containing format information
 * @param writeBuffers_pp Array of write buffers (one per channel)
 * @param bufferFillBytes_p Array tracking bytes filled in each buffer
 * @param bufferSizeBytes Size of each buffer in bytes
 * @param outputFiles_pp Array of output file handles
 * @param bytesWritten_p Array tracking bytes written per channel
 */
void extract_audio_from_chunk(FILE *inputFile_p, const WavHeader *inputHeader,
                             uint8_t **writeBuffers_pp, size_t *bufferFillBytes_p,
                             size_t bufferSizeBytes, FILE **outputFiles_pp,
                             uint32_t *bytesWritten_p);

/**
 * Flush any remaining buffered data to output files
 * 
 * @param inputHeader WAV header containing channel information
 * @param writeBuffers_pp Array of write buffers (one per channel)
 * @param bufferFillBytes_p Array tracking bytes filled in each buffer
 * @param outputFiles_pp Array of output file handles
 * @param bytesWritten_p Array tracking bytes written per channel
 */
void flush_remaining_buffers(const WavHeader *inputHeader, uint8_t **writeBuffers_pp,
                            size_t *bufferFillBytes_p, FILE **outputFiles_pp,
                            uint32_t *bytesWritten_p);

/**
 * Finalize output files by rewriting headers with correct sizes and cleanup
 * 
 * @param inputHeader WAV header containing format information
 * @param bytesWritten_p Pointer to array tracking bytes written per channel
 * @param outputFiles_pp Pointer to array of output file handles
 */
void finalize_output_files(const WavHeader *inputHeader, uint32_t **bytesWritten_p,
                          FILE ***outputFiles_pp);

#endif // PROCESSING_H
