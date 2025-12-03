# Brainfuck-net
A simple and efficient Brainfuck interpreter written in C. This project is part of building a server and client for a 1:1 chat application in Brainfuck (part of EWOR admission).


## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Building](#building)
- [Usage](#usage)
  - [Brainfuck Interpreter (cbf)](#brainfuck-interpreter-cbf)
  - [Text-to-Brainfuck Encoder (encoder)](#text-to-brainfuck-encoder-encoder)
- [Brainfuck Language Specification](#brainfuck-language-specification)
  - [Execution Model](#execution-model)
- [Implementation Details](#implementation-details)
  - [Architecture](#architecture)
  - [Error Handling](#error-handling)
  - [Memory Management](#memory-management)
- [Testing](#testing)
- [Project Goals](#project-goals)
- [Code Quality](#code-quality)
- [License](#license)
- [Note on AI Use](#note-on-ai-use)


## Overview
This project includes two main tools:

- **`cbf`**: A standard-compliant Brainfuck interpreter that executes Brainfuck programs with proper error handling, bounds checking, and a clean, modular codebase.
- **`encoder`**: A utility that converts text files into Brainfuck code, generating programs that print the exact contents of the input file.


## Features
- **Standard Brainfuck Implementation**: Implements all 8 Brainfuck commands correctly
- **Bounds Checking**: Prevents data pointer from going out of bounds
- **Error Handling**: Comprehensive error messages for common issues
- **Modular Design**: Well-organized code with clear separation of concerns
- **Memory Safety**: Proper memory management and null checks
- **Text-to-Brainfuck Encoder**: Convert any text file into Brainfuck code that reproduces the original text


## Building
The project uses a simple Makefile for building:

```bash
make              # Build both cbf and encoder executables
make clean        # Remove built artifacts
make test         # Run test suite
```

Both tools are compiled with:
- C11 standard
- `-Wall -Wextra` for comprehensive warnings
- `-O2` for optimization


## Usage
### Brainfuck Interpreter (cbf)
```bash
./cbf <filename.bf>
```

**Example:**
```bash
./cbf tests/hello.bf
# Output: Hello, World!
```

### Text-to-Brainfuck Encoder (encoder)
```bash
./encoder <input_file> [output_file]
```

The encoder reads a text file and generates a Brainfuck program (`.bf` file) that, when executed, prints the exact contents of the input file.

**Arguments:**
- `input_file`: The text file to encode
- `output_file`: (Optional) The output Brainfuck file. If not specified, the encoder automatically generates the output filename by replacing the input file's extension with `.bf`, or appending `.bf` if the file has no extension.

**Examples:**
```bash
# Using default output filename
echo "Hello, World!" > message.txt
./encoder message.txt        # Creates message.bf
./cbf message.bf            # Output: Hello, World!

# Specifying custom output filename
./encoder message.txt custom_output.bf
./cbf custom_output.bf      # Output: Hello, World!
```


## Brainfuck Language Specification
Brainfuck operates on a tape of 30,000 cells, each holding a single byte (0-255). The language consists of 8 commands:

| Command | Description |
|---------|-------------|
| `>` | Increment the data pointer (move right) |
| `<` | Decrement the data pointer (move left) |
| `+` | Increment the byte at the data pointer |
| `-` | Decrement the byte at the data pointer |
| `.` | Output the byte at the data pointer as ASCII |
| `,` | Input a byte and store it at the data pointer |
| `[` | Jump forward to matching `]` if current cell is zero |
| `]` | Jump back to matching `[` if current cell is non-zero |

### Execution Model
1. All cells are initialized to 0
2. The data pointer starts at cell 0
3. Commands execute sequentially from left to right
4. Cell values wrap around: 255 + 1 = 0, 0 - 1 = 255
5. Any non-command characters are treated as comments


## Implementation Details
### Architecture
The interpreter is structured with the following components:

- **File Reading**: Safely reads Brainfuck source files into memory
- **Interpreter State**: Manages tape, data pointer, code, and program counter
- **Instruction Execution**: Executes individual Brainfuck commands
- **Bracket Matching**: Handles loop jumps with proper error detection
- **Bounds Checking**: Ensures data pointer stays within valid tape range

### Error Handling
The interpreter provides clear error messages for:

- File I/O errors (missing files, read failures)
- Memory allocation failures
- Unmatched brackets (`[` without `]` or vice versa)
- Data pointer out of bounds
- Invalid file operations

### Memory Management
- Tape: 30,000 cells allocated dynamically with `calloc` (initialized to zero)
- Code: Read from file and stored in dynamically allocated buffer
- All resources are properly freed on exit


## Testing
The project includes test files in the `tests/` directory:

- `hello_world.bf`: Classic "Hello, World!" program
- `ascii_[animal].bf`: ASCII art demonstration

Run tests with:
```bash
./cbf [filepath].bf
```

## Project Goals
This interpreter is the foundation for building a network chat application in Brainfuck. See `braindump.md` for detailed discussion of the challenges and approaches for implementing network communication in Brainfuck.


## Code Quality
The codebase follows best practices:

- Clear function separation and single responsibility
- Comprehensive error handling
- Detailed comments explaining behavior
- Memory safety with proper bounds checking
- Standard C11 compliance


## License
See `LICENSE` file for details.

---

## Note on AI Use
Generative AI was used as a boosted search engine. Moreover, it was used to create the documentation and ensure it is clear and easy to understand.
