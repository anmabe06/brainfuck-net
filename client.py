import socket
import sys

# In server.bf: Cell value 80 * 100 = 8000
HOST = "127.0.0.1"
PORT = 8000

def read_until_newline(s):
    """
    Reads from socket 's' byte by byte until a newline is found.
    Returns the decoded string.
    """
    buf = b""
    while True:
        try:
            chunk = s.recv(1)
            if not chunk:
                return None # Connection closed
            buf += chunk
            if chunk == b'\n':
                break
        except socket.error:
            return None
    return buf.decode(errors='replace').strip()

def main():
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            print(f"Connecting to {HOST}:{PORT}...")
            s.connect((HOST, PORT))
            print("Connected! Type a message and press Enter.")

            while True:
                try:
                    # Python 3 input
                    msg = input("> ")
                except EOFError:
                    print("\nExiting...")
                    break
                
                # Send message with newline
                s.sendall((msg + "\n").encode())

                # Receive response (read until newline)
                response = read_until_newline(s)
                
                if response is None:
                    print("\nServer disconnected.")
                    break
                
                print(f"Server Echo: {response}")

    except ConnectionRefusedError:
        print(f"Error: Could not connect to {HOST}:{PORT}.")
        print("Make sure the cbf interpreter is running: ./cbf echo_server.bf")
    except KeyboardInterrupt:
        print("\nClient stopped.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()