/*
 * encoder.c - Converts a text file to Brainfuck code
 *
 * Usage: ./encoder <input_file> [output_file]
 *        If output_file is not specified, it defaults to input_file with .bf extension
 * Output: Brainfuck code that prints the contents of the input file
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 * Writes a character to the output file by generating Brainfuck code
 * that sets the current cell to the character's ASCII value and outputs it.
 */
static void encode_char(FILE *out, uint8_t c) {
    // Generate code to set current cell to the character value
    for (int i = 0; i < c; i++) {
        fputc('+', out);
    }
    
    // Output the character
    fputc('.', out);
    
    // Reset the cell to 0 for next character
    for (int i = 0; i < c; i++) {
        fputc('-', out);
    }
}

/**
 * Encodes a text file to Brainfuck code.
 * Returns 0 on success, -1 on error.
 */
static int encode_file(const char *input_filename, const char *output_filename) {
    FILE *input = fopen(input_filename, "rb");
    if (!input) {
        perror("Error opening input file");
        return -1;
    }

    FILE *output = fopen(output_filename, "w");
    if (!output) {
        perror("Error opening output file");
        fclose(input);
        return -1;
    }

    int c;
    while ((c = fgetc(input)) != EOF) {
        encode_char(output, (uint8_t)c);
    }

    fclose(input);
    fclose(output);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <input_file> [output_file]\n", argv[0]);
        fprintf(stderr, "Example: %s file.txt\n", argv[0]);
        fprintf(stderr, "Example: %s file.txt output.bf\n", argv[0]);
        fprintf(stderr, "If output_file is not specified, defaults to input_file with .bf extension\n");
        return 1;
    }

    char *input_filename = argv[1];
    char output_filename[1024];
    
    if (argc == 3) {
        // Use provided output filename
        strncpy(output_filename, argv[2], sizeof(output_filename) - 1);
        output_filename[sizeof(output_filename) - 1] = '\0';
    } else {
        // Generate output filename by replacing extension with .bf
        strncpy(output_filename, input_filename, sizeof(output_filename) - 1);
        output_filename[sizeof(output_filename) - 1] = '\0';
        
        // Find the last dot to replace extension
        char *dot = strrchr(output_filename, '.');
        if (dot) {
            strcpy(dot, ".bf");
        } else {
            // No extension, append .bf
            strncat(output_filename, ".bf", sizeof(output_filename) - strlen(output_filename) - 1);
        }
    }

    if (encode_file(input_filename, output_filename) != 0) {
        return 1;
    }

    printf("Encoded %s -> %s\n", input_filename, output_filename);
    return 0;
}

