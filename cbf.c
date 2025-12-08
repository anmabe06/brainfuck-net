/*
* cbf.c - A simple Brainfuck interpreter in C with Brainfuck-Net extension.
*
* Brainfuck-Net Extension (compile with -DBFNET):
* ^ - Server Hook: Creates a TCP server.
* Port = [Current Cell Value] * 100. (e.g., 80 -> 8000).
* % - Stream Toggle: Toggles I/O mode between console (default) and network.
*
* Usage:
* gcc cbf.c -o cbf -DBFNET
* ./cbf echo_server.bf
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#ifdef BFNET
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#endif

#define TAPE_SIZE 30000

// Debug logging utilities
#ifdef BFNET
static int enable_logging = 0;  // Set to 1 if --logging flag is present
static int enable_timestamps = 0;  // Set to 1 if --timestamps flag is present

static void print_timestamp(void) {
    if (!enable_timestamps) return;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *tm_info = localtime(&ts.tv_sec);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(stderr, "[%s.%03ld] ", buffer, ts.tv_nsec / 1000000);
}

static void log_debug(const char *format, ...) {
    if (!enable_logging) return;
    print_timestamp();
    va_list args;
    fprintf(stderr, "\033[0;33m[DEBUG]\033[0m ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

static void log_info(const char *format, ...) {
    if (!enable_logging) return;
    print_timestamp();
    va_list args;
    fprintf(stderr, "\033[0;32m[INFO]\033[0m ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

static void log_error(const char *format, ...) {
    // Error logs are always enabled
    print_timestamp();
    va_list args;
    fprintf(stderr, "\033[0;31m[ERROR]\033[0m ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

static void log_net(const char *format, ...) {
    if (!enable_logging) return;
    print_timestamp();
    va_list args;
    fprintf(stderr, "\033[0;36m[NET]\033[0m ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}
#endif

#ifdef BFNET
int net_mode = 0;        // 0 = console, 1 = network
int server_fd = -1;
int client_fd = -1;
#endif

typedef struct {
    uint8_t *tape;
    uint8_t *ptr;
    char *code;
    long code_size;
    long pc;
} Interpreter;

static char *read_file(const char *filename, long *size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *code = (char *)malloc(*size + 1);
    if (!code) {
        fclose(file);
        return NULL;
    }
    
    if (fread(code, 1, *size, file) != (size_t)*size) {
        free(code);
        fclose(file);
        return NULL;
    }
    code[*size] = '\0';
    fclose(file);
    #ifdef BFNET
    log_info("Loaded Brainfuck program: %s (%ld bytes)", filename, *size);
    #endif
    return code;
}

static long find_match(Interpreter *interp, char open, char close, int dir) {
    int depth = 1;
    long pc = interp->pc + dir;
    while (depth > 0 && pc >= 0 && pc < interp->code_size) {
        if (interp->code[pc] == open) depth++;
        else if (interp->code[pc] == close) depth--;
        if (depth > 0) pc += dir;
    }
    return (depth == 0) ? pc : -1;
}

static int execute_instruction(Interpreter *interp) {
    char instruction = interp->code[interp->pc];
    long jump_pos;
    
    switch (instruction) {
        case '>':
        if (interp->ptr < interp->tape + TAPE_SIZE - 1) interp->ptr++;
        break;
        case '<':
        if (interp->ptr > interp->tape) interp->ptr--;
        break;
        case '+': (*interp->ptr)++; break;
        case '-': (*interp->ptr)--; break;
        case '.':
        #ifdef BFNET
        if (net_mode == 1 && client_fd != -1) {
            uint8_t byte = *interp->ptr;
            ssize_t sent = send(client_fd, &byte, 1, 0);
            if (sent < 0) {
                log_error("Send failed: %s (fd=%d)", strerror(errno), client_fd);
            } else if (sent == 1) {
                log_net("SENT: byte=%d (0x%02x) '%c'", byte, byte, (byte >= 32 && byte < 127) ? byte : '.');
            } else {
                log_error("Send incomplete: %zd bytes sent (expected 1)", sent);
            }
        } else {
            if (net_mode == 1 && client_fd == -1) {
                log_error("Attempted send in net_mode but socket is invalid");
            }
            putchar(*interp->ptr);
            fflush(stdout);
        }
        #else
        putchar(*interp->ptr);
        fflush(stdout);
        #endif
        break;
        case ',':
        #ifdef BFNET
        if (net_mode == 1) {
            if (client_fd != -1) {
                log_debug("Waiting for network input (blocking)...");
                uint8_t byte;
                ssize_t n = recv(client_fd, &byte, 1, MSG_WAITALL);
                if (n > 0) {
                    *interp->ptr = byte;
                    log_net("RECV: byte=%d (0x%02x) '%c'", byte, byte, (byte >= 32 && byte < 127) ? byte : '.');
                } else if (n == 0) {
                    log_error("Connection closed by peer (EOF)");
                    *interp->ptr = 0;
                } else {
                    log_error("Recv failed: %s (fd=%d)", strerror(errno), client_fd);
                    *interp->ptr = 0;
                }
            } else {
                log_error("Attempted recv in net_mode but socket is invalid");
                *interp->ptr = 0;
            }
        } else {
            int c = getchar();
            *interp->ptr = (c == EOF) ? 0 : (uint8_t)c;
        }
        #else
        {
            int c = getchar();
            *interp->ptr = (c == EOF) ? 0 : (uint8_t)c;
        }
        #endif
        break;
        case '[':
        if (*interp->ptr == 0) {
            jump_pos = find_match(interp, '[', ']', 1);
            if (jump_pos == -1) return -1;
            interp->pc = jump_pos;
        }
        break;
        case ']':
        if (*interp->ptr != 0) {
            jump_pos = find_match(interp, ']', '[', -1);
            if (jump_pos == -1) return -1;
            interp->pc = jump_pos;
        }
        break;
        #ifdef BFNET
        case '^': {
            int port = (*interp->ptr) * 100;
            log_info("SERVER: Initializing server on port %d (cell value: %d)", port, *interp->ptr);
            
            server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd < 0) {
                log_error("Socket creation failed: %s", strerror(errno));
                *interp->ptr = 0;
                break;
            }
            log_debug("Socket created: fd=%d", server_fd);
            
            int opt = 1;
            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                log_error("setsockopt(SO_REUSEADDR) failed: %s", strerror(errno));
            }
            
            struct sockaddr_in addr = {0};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = INADDR_ANY;
            
            if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                log_error("Bind failed on port %d: %s", port, strerror(errno));
                close(server_fd);
                server_fd = -1;
                *interp->ptr = 0;
            } else {
                log_info("Bound to 0.0.0.0:%d", port);
                if (listen(server_fd, 1) < 0) {
                    log_error("Listen failed: %s", strerror(errno));
                    close(server_fd);
                    server_fd = -1;
                    *interp->ptr = 0;
                } else {
                    log_info("Listening for connections (backlog=1)...");
                    log_debug("Waiting for client to connect (blocking accept)...");
                    client_fd = accept(server_fd, NULL, NULL);
                    
                    if (client_fd >= 0) {
                        int flag = 1;
                        if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) < 0) {
                            log_error("setsockopt(TCP_NODELAY) failed: %s", strerror(errno));
                        } else {
                            log_debug("TCP_NODELAY enabled for low-latency communication");
                        }
                        log_info("Client connected! (fd=%d)", client_fd);
                    } else {
                        log_error("Accept failed: %s", strerror(errno));
                        *interp->ptr = 0;
                    }
                }
            }
            break;
        }
        case '&': {
            int port = (*interp->ptr) * 100;
            log_info("CLIENT: Connecting to 127.0.0.1:%d (cell value: %d)", port, *interp->ptr);
            
            client_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (client_fd < 0) {
                log_error("Socket creation failed: %s", strerror(errno));
                *interp->ptr = 0;
                break;
            }
            log_debug("Socket created: fd=%d", client_fd);
            
            struct sockaddr_in addr = {0};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            
            log_debug("Attempting connection to 127.0.0.1:%d...", port);
            if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                log_error("Connect failed to 127.0.0.1:%d: %s", port, strerror(errno));
                close(client_fd);
                client_fd = -1;
                *interp->ptr = 0;
            } else {
                int flag = 1;
                if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) < 0) {
                    log_error("setsockopt(TCP_NODELAY) failed: %s", strerror(errno));
                } else {
                    log_debug("TCP_NODELAY enabled for low-latency communication");
                }
                log_info("Connected to server! (fd=%d)", client_fd);
            }
            break;
        }
        case '%':
            net_mode = !net_mode;
            log_info("Mode switched to: %s (fd=%d)", net_mode ? "NETWORK" : "CONSOLE", client_fd);
            break;
        #endif
    }
    return 0;
}

int main(int argc, char *argv[]) {
    const char *filename = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--logging") == 0) {
            #ifdef BFNET
            enable_logging = 1;
            #endif
        } else if (strcmp(argv[i], "--timestamps") == 0) {
            #ifdef BFNET
            enable_timestamps = 1;
            #endif
        } else if (filename == NULL) {
            filename = argv[i];
        } else {
            fprintf(stderr, "Usage: %s [--logging] [--timestamps] <filename.bf>\n", argv[0]);
            return 1;
        }
    }
    
    if (filename == NULL) {
        fprintf(stderr, "Usage: %s [--logging] [--timestamps] <filename.bf>\n", argv[0]);
        return 1;
    }
    
    #ifdef BFNET
    log_info("Brainfuck-Net Interpreter starting...");
    #endif
    
    long size;
    char *code = read_file(filename, &size);
    if (!code) return 1;
    
    Interpreter interp = {0};
    interp.tape = (uint8_t *)calloc(TAPE_SIZE, 1);
    if (!interp.tape) {
        fprintf(stderr, "Failed to allocate tape memory\n");
        free(code);
        return 1;
    }
    interp.ptr = interp.tape;
    interp.code = code;
    interp.code_size = size;
    
    #ifdef BFNET
    log_debug("Interpreter initialized: tape_size=%d, code_size=%ld", TAPE_SIZE, size);
    #endif
    
    while (interp.pc < interp.code_size) {
        if (execute_instruction(&interp) != 0) {
            #ifdef BFNET
            log_error("Execution error at PC=%ld", interp.pc);
            #endif
            break;
        }
        interp.pc++;
    }
    
    #ifdef BFNET
    log_info("Program execution completed");
    #endif
    
    if (interp.tape) free(interp.tape);
    free(code);
    #ifdef BFNET
    if (client_fd != -1) {
        log_debug("Closing client socket (fd=%d)", client_fd);
        close(client_fd);
    }
    if (server_fd != -1) {
        log_debug("Closing server socket (fd=%d)", server_fd);
        close(server_fd);
    }
    log_info("Cleanup complete, exiting");
    #endif
    return 0;
}