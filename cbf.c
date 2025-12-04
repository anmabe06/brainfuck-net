/*
 * cbf.c - A simple Brainfuck interpreter in C with Brainfuck-Net extension.
 *
 * Brainfuck-Net Extension (compile with -DBFNET):
 * ^ - Server Hook: Creates a TCP server.
 * Port = [Current Cell Value] * 100. (e.g., 80 -> 8000).
 * % - Stream Toggle: Toggles I/O mode between console (default) and network.
 * ! - Async I/O Poll: Peeks at network socket. (Not used in this echo server)
 *
 * Usage:
 * gcc cbf.c -o cbf -DBFNET
 * ./cbf echo_server.bf
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <string.h>
 
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
     if (!code) return NULL;
 
     if (fread(code, 1, *size, file) != (size_t)*size) {
         free(code);
         fclose(file);
         return NULL;
     }
     code[*size] = '\0';
     fclose(file);
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
                 // Send with 0 flags. TCP_NODELAY (set in ^) handles the latency.
                 send(client_fd, interp->ptr, 1, 0);
             } else {
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
                     // Blocking receive
                     // If connection closes or error, we set cell to 0
                     int n = recv(client_fd, interp->ptr, 1, MSG_WAITALL);
                     if (n <= 0) *interp->ptr = 0; 
                 } else {
                     *interp->ptr = 0;
                 }
             } else {
                 int c = getchar();
                 *interp->ptr = (c == EOF) ? 0 : (uint8_t)c;
             }
 #else
             int c = getchar();
             *interp->ptr = (c == EOF) ? 0 : (uint8_t)c;
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
             server_fd = socket(AF_INET, SOCK_STREAM, 0);
             
             int opt = 1;
             setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
 
             struct sockaddr_in addr = {0};
             addr.sin_family = AF_INET;
             addr.sin_port = htons(port);
             addr.sin_addr.s_addr = INADDR_ANY;
 
             if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                 perror("Bind failed");
                 *interp->ptr = 0; // Signal failure
             } else {
                 listen(server_fd, 1);
                 printf("Listening on port %d...\n", port);
                 client_fd = accept(server_fd, NULL, NULL);
                 
                 if (client_fd >= 0) {
                     // ENABLE TCP_NODELAY
                     // This is critical for Brainfuck which sends data byte-by-byte.
                     // Without this, the OS buffers output, causing the "lag" you saw.
                     int flag = 1;
                     setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
                     printf("Client connected!\n");
                 }
             }
             break;
         }
         case '%':
             net_mode = !net_mode;
             break;
         case '!': {
             if (client_fd == -1) { *interp->ptr = 0; break; }
             unsigned char c;
             int n = recv(client_fd, &c, 1, MSG_DONTWAIT | MSG_PEEK);
             if (n > 0) *interp->ptr = c;
             else *interp->ptr = 0;
             break;
         }
 #endif
     }
     return 0;
 }
 
 int main(int argc, char *argv[]) {
     if (argc != 2) {
         fprintf(stderr, "Usage: %s <filename.bf>\n", argv[0]);
         return 1;
     }
 
     long size;
     char *code = read_file(argv[1], &size);
     if (!code) return 1;
 
     Interpreter interp = {0};
     interp.tape = (uint8_t *)calloc(TAPE_SIZE, 1);
     interp.ptr = interp.tape;
     interp.code = code;
     interp.code_size = size;
 
     while (interp.pc < interp.code_size) {
         if (execute_instruction(&interp) != 0) break;
         interp.pc++;
     }
 
     if (interp.tape) free(interp.tape);
     free(code);
     #ifdef BFNET
     if (client_fd != -1) close(client_fd);
     if (server_fd != -1) close(server_fd);
     #endif
     return 0;
 }