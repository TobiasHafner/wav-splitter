#include <stdio.h>
#include "wav-splitter.h"

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input WAV file>\n", argv[0]);
        return 1;
    }

    FILE *inputFile = fopen(argv[1], "rb");
    if (!inputFile) {
        perror("ERROR: Failed to open input file");
        return 1;
    }

    struct WavHeader header;
    if (read_header(inputFile, &header) != 0) {
        fclose(inputFile);
        return 1;
    }

    if (split_wav_file(inputFile, &header, argv[1]) != 0) {
        fclose(inputFile);
        return 1;
    }

    fclose(inputFile);

    printf("WAV splitting completed successfully.\n");
    return 0;
}
