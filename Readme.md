# Brainfuck-net
A simple and efficient Brainfuck interpreter written in C. This project is part of building a server and client for a 1:1 chat application in Brainfuck (part of EWOR admission).


## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Building](#building)
- [Usage](#usage)
  - [Brainfuck Interpreter (cbf)](#brainfuck-interpreter-cbf)
  - [Brainfuck-Net Network Extension](#brainfuck-net-network-extension)
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
- **Brainfuck-Net Extension**: Network support for building server/client applications (compile with `-DBFNET`)
- **Bounds Checking**: Prevents data pointer from going out of bounds
- **Error Handling**: Comprehensive error messages for common issues
- **Modular Design**: Well-organized code with clear separation of concerns
- **Memory Safety**: Proper memory management and null checks
- **Text-to-Brainfuck Encoder**: Convert any text file into Brainfuck code that reproduces the original text


## Building
The project uses a simple Makefile for building:

```bash
make              # Build both cbf and encoder executables (standard Brainfuck)
make net          # Build cbf with Brainfuck-Net network support
make clean        # Remove built artifacts
make test         # Run test suite
```

Both tools are compiled with:
- C11 standard
- `-Wall -Wextra` for comprehensive warnings
- `-O2` for optimization

**Note**: To use network features, you must build with `make net` which compiles with the `-DBFNET` flag.


## Usage
### Brainfuck Interpreter (cbf)
```bash
./cbf <filename.bf>
```

**Example:**
```bash
./cbf tests/hello_world.bf
# Output: Hello, World!
```

### Brainfuck-Net Network Extension

When compiled with `make net`, the interpreter supports network operations through three additional commands:

| Command | Description |
|---------|-------------|
| `^` | **Server Hook**: Creates a TCP server and accepts a connection. Port = [Current Cell Value] × 100 (e.g., cell value 80 → port 8000) |
| `%` | **Stream Toggle**: Toggles I/O mode between console (default) and network. When in network mode, `.` sends to socket and `,` reads from socket |
| `!` | **Async I/O Poll**: Non-blocking peek at network socket. Sets current cell to the next available byte, or 0 if no data available |

**Network Mode Behavior:**
- When network mode is **off** (default): `.` outputs to stdout, `,` reads from stdin
- When network mode is **on**: `.` sends bytes to the connected client, `,` receives bytes from the client
- If a connection closes or read fails, `,` sets the current cell to 0

**Example: Echo Server**

Create a simple echo server that listens on port 8000:

```bash
# Build with network support
make net

# Run the echo server
./cbf tests/echo_server.bf
# Output: Listening on port 8000...
#         Client connected!
```

The server will echo back any data sent to it. You can test it using the provided Python client:

```bash
# In another terminal
python3 client.py
```

**Example: Building a Server**

Here's how to create a server in Brainfuck:

```brainfuck
[-]                 Clear Cell 0
>++++++++           Cell 1 = 8
[<++++++++++>-]     Cell 0 = 80 (8 * 10)
<                   Move back to Cell 0 (Value 80)
^                   Start Server on Port 8000
%                   Switch to Network Mode

>                   Move to Cell 1
+                   Set Cell 1 to 1 to enter the loop initially
[                   MAIN LOOP
    ,               Read byte from socket (Waits for data)
    .               Echo byte back to socket
]                   If read failed (connection closed), cell is 0, loop exits
```

See `tests/echo_server.bf` for a complete working example.

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
- `echo_server.bf`: Network echo server example (requires `make net`)

Run tests with:
```bash
./cbf [filepath].bf
```

**Testing Network Features:**

1. Build with network support:
   ```bash
   make net
   ```

2. Start the echo server:
   ```bash
   ./cbf tests/echo_server.bf
   ```

3. In another terminal, connect using the Python client:
   ```bash
   python3 client.py
   ```

The client will connect to `localhost:8000` and allow you to send messages that the server will echo back.

## Project Goals
This interpreter is the foundation for building a network chat application in Brainfuck. The Brainfuck-Net extension adds minimal network primitives (`^`, `%`, `!`) that enable server/client applications while maintaining Brainfuck's minimalist philosophy. See `braindump.md` for detailed discussion of the challenges and approaches for implementing network communication in Brainfuck.


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
