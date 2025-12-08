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
This project includes two main tools and great code example:

- **`cbf`**: A Custom Brainfuck Interpreter that executes Brainfuck programs with proper error handling, bounds checking, and a clean, modular codebase. It is custom, because it enables the execution of programs written in Brainfuck-net (see below).
- **`encoder`**: A utility that converts text files into Brainfuck code, generating programs that print the exact contents of the input file.
- **`examples`** for simple ascii-producing programs, echo server-client scripts, and scripts to create a 1:1 chatroom


## Features
- **Standard Brainfuck Implementation**: Implements all 8 Brainfuck commands correctly
- **Brainfuck-Net Extension**: Network support for building server/client applications (compile with `-DBFNET`)
- **Debug Logging**: Optional detailed logging with `--logging` flag (Brainfuck-Net only)
- **Timestamped Logs**: Optional timestamps in log messages with `--timestamps` flag (Brainfuck-Net only)
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
./cbf [--logging] [--timestamps] <filename.bf>
```

**Command-line Options:**
- `--logging`: (Brainfuck-Net only) Enables detailed logging output to stderr, including debug, info, and network messages
- `--timestamps`: (Brainfuck-Net only) Adds timestamps to log messages when used with `--logging`
- `<filename.bf>`: The Brainfuck program file to execute

**Note:** The `--logging` and `--timestamps` flags are only available when the interpreter is compiled with `make net` (Brainfuck-Net extension).

**Example:**
```bash
# Basic usage
./cbf tests/simple/hello_world.bf
# Output: Hello, World!

# With logging (Brainfuck-Net only)
./cbf --logging tests/simple/hello_world.bf

# With logging and timestamps (Brainfuck-Net only)
./cbf --logging --timestamps tests/simple/hello_world.bf
```

### Brainfuck-Net Network Extension

When compiled with `make net`, the interpreter supports network operations through four additional commands:

| Command | Description |
|---------|-------------|
| `^` | **Server Hook**: Starts a TCP server listening on a port defined by the current cell value. The port is calculated as: `port = current_cell * 100` (e.g., cell value 80 → port 8000) |
| `&` | **Client Hook**: Connects to a TCP server. The port is calculated the same way: `port = current_cell * 100` |
| `%` | **Stream Toggle**: Toggles I/O mode between **Console** (default) and **Network**. When in network mode, `.` sends to socket and `,` reads from socket |

**Mode-Based I/O:**

The extension changes how the standard Brainfuck I/O commands work depending on the network mode (toggled via `%`):

**Console Mode (default):**
- **`.` (output)**: Writes the current cell value to **stdout**
- **`,` (input)**: Reads a character from **stdin**. If EOF occurs, the cell is set to `0`

**Network Mode:**
- **`.` (output)**: Sends the current cell byte to the connected TCP socket. `TCP_NODELAY` is enabled to remove buffering latency
- **`,` (input)**: Reads a byte from the socket (blocking). If the connection closes or errors, the cell becomes `0`

**Example: Echo Server**

Create a simple echo server that listens on port 8000:

```bash
# Build with network support
make net

# Run the echo server with logging enabled
./cbf --logging tests/echo_server/echo_server.bf
# Output: [INFO] Brainfuck-Net Interpreter starting...
#         [INFO] Loaded Brainfuck program: ./tests/echo_server/echo_server.bf (558 bytes)
#         [DEBUG] Interpreter initialized: tape_size=30000, code_size=558
#         [INFO] SERVER: Initializing server on port 8000 (cell value: 80)
#         [DEBUG] Socket created: fd=3
#         [INFO] Bound to 0.0.0.0:8000
#         [INFO] Listening for connections (backlog=1)...
#         [DEBUG] Waiting for client to connect (blocking accept)
```

The server will echo back any data sent to it. You can test it using the provided Brainfuck client:

```bash
# In another terminal (with logging to see network activity)
./cbf --logging tests/echo_server/echo_client.bf
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

See `tests/echo_server/echo_server.bf` for a complete working example.

**Example: Building a Client**

Here's how to create a client in Brainfuck:

```brainfuck
[-]                 Clear Cell 0
>++++++++           Cell 1 = 8
[<++++++++++>-]     Cell 0 = 80 (8 * 10)
<                   Move back to Cell 0 (Value 80)
&                   Connect to Server on Port 8000
%                   Switch to Network Mode

>                   Move to Cell 1
[                   MAIN LOOP
    ,               Read byte from stdin
    .               Send byte to server
    ,               Read response from server
    %               Switch to Console Mode
    .               Print response to stdout
    %               Switch back to Network Mode
]
```

See `tests/echo_server/echo_client.bf` for a complete working example.

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

- `simple/hello_world.bf`: Classic "Hello, World!" program
- `simple/test_input.bf`: Input handling demonstration
- `ascii/*.bf`: ASCII art demonstrations
- `echo_server/echo_server.bf`: Network echo server example (requires `make net`)
- `echo_server/echo_client.bf`: Network echo client example (requires `make net`)
- `chatroom/*.bf`: Multi-peer chatroom examples (requires `make net`)

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
   ./cbf --logging tests/echo_server/echo_server.bf
   ```

3. In another terminal, connect using the Brainfuck client:
   ```bash
   ./cbf --logging tests/echo_server/echo_client.bf
   ```

The client will connect to `localhost:8000` and allow you to send messages that the server will echo back.

## Project Goals
This interpreter is the foundation for building a network chat application in Brainfuck. The Brainfuck-Net extension adds minimal network primitives (`^`, `&`, `%`) that enable server/client applications while maintaining Brainfuck's minimalist philosophy. The extension works by overloading the standard I/O commands (`.` and `,`) depending on a mode flag, allowing seamless switching between console and network I/O. See `braindump.md` for detailed discussion of the challenges and approaches for implementing network communication in Brainfuck.


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
