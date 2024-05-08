#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_PATH_LENGTH 250

#ifdef WIN32
#include <windows.h>
#define PATH_SEPARATOR '\\'
#else
#include <dirent.h>
#include <sys/types.h>
#define PATH_SEPARATOR '/'
#endif

#include "processing-utils.h"

void _cleanup(FILE ***outputFiles, const uint16_t count, uint32_t **bytesWritten) {
    if (outputFiles) {
        for (int i = 0; i < count + 1; i++) {
            if ((*outputFiles)[i]) fclose((*outputFiles)[i]);
        }
        free(*outputFiles);
    }
    free(*bytesWritten);
}

void _create_output_folder(char *outputPath) {
#ifdef WIN32
    if (CreateDirectory(outputPath, NULL) == 0) {
        // CreateDirectory returns 0 on fail
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            fprintf(stderr, "Failed to create directory: %s\n", outputPath);
            exit(-1);
        }
        fprintf(stderr, "Directory already exists: %s\n", outputPath);
    }
#else
    if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
        perror("Failed to create directory");
        exit(-1);
    }
#endif
}

void _find_max_chunk_index(uint64_t *maxChunkIndex, const char *sessionPath) {
    uint64_t currentChunkIndex = 0;
    char currentHexIndex[9] = {0};

#ifdef WIN32
    char searchPath[MAX_PATH_LENGTH];
    snprintf(searchPath, sizeof(searchPath), "%s\\*.*", sessionPath);
    searchPath[MAX_PATH_LENGTH - 1] = '\0';

    WIN32_FIND_DATA currentEntry;
    HANDLE directoryHandle = FindFirstFile(searchPath, &currentEntry);
    if (directoryHandle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to open the directory.");
        exit(1);
    }

    do {
        if (!strstr(currentEntry.cFileName, ".WAV")) {
            continue;
        }
        strncpy(currentHexIndex, currentEntry.cFileName, strlen(currentEntry.cFileName) - 4);
        currentHexIndex[8] = '\0';
        if (sscanf(currentHexIndex, "%lx", &currentChunkIndex) != 1) {
            continue;
        }
        if (currentChunkIndex > *maxChunkIndex) {
            *maxChunkIndex = currentChunkIndex;
        }
    } while (FindNextFile(directoryHandle, &currentEntry));

    FindClose(directoryHandle);

#else
    const char *dirPath = argv[1];
    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        fprintf(stderr, "Failed to open the session directory.");
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!strstr(entry->d_name, ".WAV") && !strstr(entry->d_name, ".wav")) {
            continue;
        }

        sscanf(entry->d_name, "lok %s", currentHexId);
        currentHexId[strlen(currentHexId) - 4] = '\0';
        sscanf(currentHexId, "%lx", &currentChunkId);

        if (currentChunkId > *maxChunkId) {
            *maxChunkId = currentChunkId;
        }
    }

    closedir(dir);
#endif
}

void _init_output_files(FILE *inputFile, FILE ***outputFiles, const WavHeader *inputHeader, uint32_t **dataWritten,
                        char *outputPath) {
    // Allocate arrays for files and bytes written
    *outputFiles = malloc(inputHeader->num_channels * sizeof(FILE *));
    *dataWritten = calloc(inputHeader->num_channels, sizeof(uint32_t));
    if (!*outputFiles || !*dataWritten) {
        fprintf(stderr, "Failed to allocate memory for output files or tracking data.\n");
        fclose(inputFile);
        _cleanup(outputFiles, inputHeader->num_channels, dataWritten);
        exit(1);
    }

    WavHeader outHeader = *inputHeader;
    outHeader.num_channels = 1;
    outHeader.byte_rate = outHeader.sample_rate * outHeader.bits_per_sample / 8;
    outHeader.block_align = outHeader.bits_per_sample / 8;
    outHeader.data_bytes = 0; // Placeholder
    outHeader.wav_size = 36 + outHeader.data_bytes;

    // create output files and write headers
    for (int i = 0; i < inputHeader->num_channels; i++) {
        char outputFileName[260];
        snprintf(outputFileName, sizeof(outputFileName), "%sch_%d.wav", outputPath, i + 1);
        (*outputFiles)[i] = fopen(outputFileName, "wb+");
        if (!(*outputFiles)[i]) {
            fprintf(stderr, "Failed to open output file %s\n", outputFileName);
            fclose(inputFile);
            _cleanup(outputFiles, i, dataWritten); // Clean up only the opened files so far
            exit(1);
        }

        if (write_header((*outputFiles)[i], &outHeader) == -1) {
            fprintf(stderr, "Failed to write header to output file.\n");
            fclose(inputFile);
            _cleanup(outputFiles, i, dataWritten);
            exit(1);
        }
    }
}

void _rewrite_headers(const WavHeader *inputHeader, uint32_t **dataWritten, FILE ***outputFiles) {
    for (int i = 0; i < inputHeader->num_channels; i++) {
        fseek((*outputFiles)[i], 0, SEEK_SET);
        WavHeader finalHeader = *inputHeader;
        finalHeader.num_channels = 1;
        finalHeader.byte_rate = finalHeader.sample_rate * finalHeader.bits_per_sample / 8;
        finalHeader.block_align = finalHeader.bits_per_sample / 8;
        finalHeader.data_bytes = (*dataWritten)[i];
        finalHeader.wav_size = 36 + finalHeader.data_bytes;

        if (write_header((*outputFiles)[i], &finalHeader) == -1) {
            fprintf(stderr, "Failed to rewrite header with correct sizes for output file %d.\n", i + 1);
        }
    }
}
