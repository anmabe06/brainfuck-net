# Brainfuck Interpreter Makefile
#
# Builds the cbf (Brainfuck interpreter) and encoder executables.
#
# Targets:
#   make        - Build cbf and encoder executables (without networking)
#   make net     - Build cbf with Brainfuck-Net networking support
#   make clean   - Remove built artifacts
#   make test    - Run tests with hello.bf and error cases
#
# Usage:
#   make
#   ./cbf tests/hello.bf
#   make net
#   ./cbf tests/server.bf
#   ./encoder file.txt

CC = cc
CFLAGS = -std=c11 -Wall -Wextra -O2
TARGET = cbf
ENCODER = encoder
SRC = cbf.c
ENCODER_SRC = encoder.c

.PHONY: all clean test net

all: $(TARGET) $(ENCODER)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

net: $(SRC)
	$(CC) $(CFLAGS) -DBFNET -o $(TARGET) $(SRC)

$(ENCODER): $(ENCODER_SRC)
	$(CC) $(CFLAGS) -o $(ENCODER) $(ENCODER_SRC)

clean:
	rm -f $(TARGET) $(ENCODER)

test: $(TARGET)
	@echo "Testing with hello.bf..."
	./$(TARGET) tests/hello.bf
	@echo ""
	@echo "Testing bracket error detection (unmatched '[')..."
	@./$(TARGET) tests/bad_brackets.bf || true
	@echo ""
	@echo "Testing bracket error detection (unexpected ']')..."
	@./$(TARGET) tests/bad_brackets2.bf || true

