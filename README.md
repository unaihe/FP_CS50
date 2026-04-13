# C-SIMPLEHTTP: A MULTIPROCESS WEB SERVER FROM SCRATCH
#### Video Demo: 
#### Description:

## Project Overview
**C-SimpleHTTP** is a lightweight, high-performance web server built from the ground up using the **C programming language**. The core objective of this project was to understand the fundamental mechanics of the World Wide Web by re-implementing the HTTP/1.1 protocol and the underlying socket communication that powers modern servers like Apache or Nginx.

Unlike higher-level languages that use pre-built libraries for networking, this project interacts directly with the Linux Kernel via the **Socket API**. It handles raw TCP connections, manages system processes, and performs manual string parsing to serve static web content to any modern browser.

## Features
- **Multiprocessing Architecture**: Utilizes the `fork()` system call to handle multiple simultaneous client connections.
- **Dynamic URL Routing**: Parses incoming HTTP `GET` requests to serve specific files based on the browser's request.
- **Robust Error Handling**: Implements a custom 404 "Not Found" response for invalid paths.
- **Clean Process Management**: Uses signal handling to prevent zombie processes and ensure OS stability.
- **Modern CSS Integration**: Includes a responsive and styled user interface to demonstrate successful content delivery.

## File Structure & Components
The project consists of three primary files, each serving a critical role in the application:

1. **`server_http.c`**: This is the heart of the project. It contains approximately 140 lines of C code that manage the entire lifecycle of a server:
   - **Initialization**: Sets up `getaddrinfo` and creates a socket.
   - **The Loop**: An infinite `while(1)` loop that waits for connections using `accept()`.
   - **The Worker (Child Process)**: Each request is handled by a child process created with `fork()`. This child reads the request into a buffer, parses the filename using `sscanf`, and performs file I/O to send the data back to the client.

2. **`index.html`**: The default landing page. It is styled with modern CSS (Flexbox, shadows, and transitions) to show that the server can handle complex text-based files and correctly set `Content-Type` and `Content-Length` headers.

3. **`contacto.html`**: A secondary page used to verify the server's dynamic routing. When a user clicks a link in the index, the server must identify the new path and open the correct file from the disk.

## Technical Design Choices

### Why Multiprocessing?
One of the key decisions was using `fork()` instead of a single-threaded model. In a single-threaded server, if one user requests a very large file, all other users are blocked until that transfer is finished. By using `fork()`, the Operating System handles the scheduling of multiple processes, allowing the server to be responsive to many users at once.

### Memory Management
I implemented manual memory allocation using `malloc` to load the HTML files into RAM. The server first uses `fseek` and `ftell` to calculate the exact size of the file on disk, allocates that specific amount of memory, and then reads the content. This ensures that the server doesn't waste RAM and can handle files of varying sizes.

### HTTP Protocol Compliance
To make the server work with browsers like Chrome or Firefox, I had to manually construct HTTP headers. This includes the status line (e.g., `HTTP/1.1 200 OK`) and headers like `Content-Length`, which tells the browser exactly how many bytes to expect. The separation between headers and body with a double carriage return/line feed (`\r\n\r\n`) was a critical detail for protocol compliance.

## Challenges and Learning
The biggest challenge was managing the "Zombie Processes." When a child process finishes sending a file, it enters a "defunct" state. If not handled, these would eventually crash the system. I solved this by implementing a `sigchld_handler` using the `sigaction` struct, which automatically "reaps" finished children in the background.

## Credits and References
This project was a deep dive into systems programming, and I relied on the following resources:
- **Beej's Guide to Network Programming**: The definitive guide for implementing the Linux Socket API (`socket`, `bind`, `listen`, `accept`).
- **CS50x Curriculum**: For the foundation in C, memory management, and file I/O.
- **Gemini AI**: Used as a technical assistant for debugging architectural issues and formatting HTTP headers correctly.

## How to Run
1. Open a Linux terminal (Kali, Ubuntu, or WSL).
2. Compile the server: `gcc server_http.c -o server`
3. Execute the binary: `./server`
4. Open a browser and navigate to `http://localhost:3490`

---
**Author:** Unai Herreo  
