/*
 * cbf.c - A simple Brainfuck interpreter in C.
 *
 * This interpreter implements the standard Brainfuck language specification
 * with a tape of 30,000 cells, each holding a single byte (0-255).
 *
 * Usage: ./cbf <filename.bf>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TAPE_SIZE 30000

/* Structure to hold interpreter state */
typedef struct {
    uint8_t *tape;
    uint8_t *ptr;
    char *code;
    long code_size;
    long pc;
} Interpreter;

/**
 * Reads a file into memory and returns its contents.
 * Returns NULL on error.
 */
static char *read_file(const char *filename, long *size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    // Determine file size
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Error seeking to end of file");
        fclose(file);
        return NULL;
    }

    *size = ftell(file);
    if (*size < 0) {
        perror("Error determining file size");
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        perror("Error seeking to start of file");
        fclose(file);
        return NULL;
    }

    // Allocate memory for the code
    char *code = (char *)malloc(*size + 1);
    if (!code) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read the file into memory
    size_t read_size = fread(code, 1, *size, file);
    if (read_size != (size_t)*size) {
        fprintf(stderr, "Error reading file: expected %ld bytes, got %zu\n", 
                *size, read_size);
        free(code);
        fclose(file);
        return NULL;
    }

    code[*size] = '\0'; // Null-terminate for safety
    fclose(file);
    return code;
}

/**
 * Finds the matching closing bracket ']' for an opening bracket '['.
 * Returns the position of the matching bracket, or -1 on error.
 */
static long find_matching_close(Interpreter *interp) {
    int depth = 1;
    long pc = interp->pc + 1;

    while (depth > 0 && pc < interp->code_size) {
        if (interp->code[pc] == '[') {
            depth++;
        } else if (interp->code[pc] == ']') {
            depth--;
        }
        if (depth > 0) {
            pc++;
        }
    }

    if (depth != 0) {
        fprintf(stderr, "Error: Unmatched '[' at position %ld\n", interp->pc);
        return -1;
    }

    return pc;
}

/**
 * Finds the matching opening bracket '[' for a closing bracket ']'.
 * Returns the position of the matching bracket, or -1 on error.
 */
static long find_matching_open(Interpreter *interp) {
    int depth = 1;
    long pc = interp->pc - 1;

    while (depth > 0 && pc >= 0) {
        if (interp->code[pc] == ']') {
            depth++;
        } else if (interp->code[pc] == '[') {
            depth--;
        }
        if (depth > 0) {
            pc--;
        }
    }

    if (depth != 0) {
        fprintf(stderr, "Error: Unmatched ']' at position %ld\n", interp->pc);
        return -1;
    }

    return pc;
}

/**
 * Checks if the data pointer is within valid bounds.
 * Returns 1 if valid, 0 if out of bounds.
 */
static int is_ptr_valid(Interpreter *interp) {
    uint8_t *tape_start = interp->tape;
    uint8_t *tape_end = interp->tape + TAPE_SIZE;
    return (interp->ptr >= tape_start && interp->ptr < tape_end);
}

/**
 * Executes a single Brainfuck instruction.
 * Returns 0 on success, -1 on error.
 */
static int execute_instruction(Interpreter *interp) {
    char instruction = interp->code[interp->pc];
    long jump_pos;

    switch (instruction) {
        case '>':
            // Move data pointer to the right
            interp->ptr++;
            if (!is_ptr_valid(interp)) {
                fprintf(stderr, "Error: Data pointer out of bounds (moved beyond tape)\n");
                return -1;
            }
            break;

        case '<':
            // Move data pointer to the left
            interp->ptr--;
            if (!is_ptr_valid(interp)) {
                fprintf(stderr, "Error: Data pointer out of bounds (moved before tape start)\n");
                return -1;
            }
            break;

        case '+':
            // Increment the byte at the data pointer (wraps around at 255)
            (*interp->ptr)++;
            break;

        case '-':
            // Decrement the byte at the data pointer (wraps around at 0)
            (*interp->ptr)--;
            break;

        case '.':
            // Output the byte at the data pointer as an ASCII character
            putchar(*interp->ptr);
            fflush(stdout); // Ensure output is flushed
            break;

        case ',':
            // Input a byte and store it at the data pointer
            {
                int c = getchar();
                if (c != EOF) {
                    *interp->ptr = (uint8_t)c;
                } else {
                    // On EOF, set cell to 0 (common Brainfuck convention)
                    *interp->ptr = 0;
                }
            }
            break;

        case '[':
            // Jump forward past the matching ']' if the byte at the data pointer is zero
            if (*interp->ptr == 0) {
                jump_pos = find_matching_close(interp);
                if (jump_pos == -1) {
                    return -1;
                }
                interp->pc = jump_pos;
            }
            break;

        case ']':
            // Jump backward to the matching '[' if the byte at the data pointer is nonzero
            if (*interp->ptr != 0) {
                jump_pos = find_matching_open(interp);
                if (jump_pos == -1) {
                    return -1;
                }
                interp->pc = jump_pos;
            }
            break;

        default:
            // Ignore any non-command characters (treat as comments)
            break;
    }

    return 0;
}

/**
 * Initializes a new Brainfuck interpreter.
 * Returns 0 on success, -1 on error.
 */
static int init_interpreter(Interpreter *interp, char *code, long code_size) {
    interp->tape = (uint8_t *)calloc(TAPE_SIZE, sizeof(uint8_t));
    if (!interp->tape) {
        fprintf(stderr, "Memory allocation failed for tape\n");
        return -1;
    }

    interp->ptr = interp->tape;
    interp->code = code;
    interp->code_size = code_size;
    interp->pc = 0;

    return 0;
}

/**
 * Runs the Brainfuck program.
 * Returns 0 on success, -1 on error.
 */
static int run_program(Interpreter *interp) {
    while (interp->pc < interp->code_size) {
        if (execute_instruction(interp) != 0) {
            return -1;
        }
        interp->pc++;
    }

    return 0;
}

/**
 * Cleans up interpreter resources.
 */
static void cleanup_interpreter(Interpreter *interp) {
    if (interp->tape) {
        free(interp->tape);
    }
}

int main(int argc, char *argv[]) {
    // Check for correct usage
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        fprintf(stderr, "Example: %s tests/hello.bf\n", argv[0]);
        return 1;
    }

    // Read the Brainfuck source file
    long code_size;
    char *code = read_file(argv[1], &code_size);
    if (!code) {
        return 1;
    }

    // Initialize interpreter
    Interpreter interp;
    if (init_interpreter(&interp, code, code_size) != 0) {
        free(code);
        return 1;
    }

    // Execute the program
    int result = run_program(&interp);

    // Clean up
    cleanup_interpreter(&interp);
    free(code);

    return (result == 0) ? 0 : 1;
}
