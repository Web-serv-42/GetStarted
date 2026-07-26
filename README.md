*This project has been created as part of the 42 curriculum by abnsila, hwahmane, ablabib.*

## Description

Webserv is an HTTP/1.0 compliant web server written in C++98. The objective of the project is to build a custom, event-driven web server from scratch to gain a deep understanding of network sockets, non-blocking I/O multiplexing, and HTTP message processing.

Key Features:

* Event-driven I/O multiplexing (`epoll`) handling non-blocking sockets.
* Full HTTP/1.0 request parsing and status code handling (`GET`, `POST`, `DELETE`).
* Nginx-style configuration file parsing supporting multiple virtual hosts, custom roots, index files, and autoindex directory listings.
* Dynamic content processing using CGI scripts (e.g., PHP, Python).
* File upload handling and multipart content type.

## Instructions

### Compilation

Compile the project using the included `Makefile`:

```bash
make
or
make re

```

### Execution

Run the server executable by passing a configuration file path:

```bash
./webserv [path/to/config.conf]

```

If no configuration file is specified, the server will fall back to its default configuration:

```bash
./webserv configs/hmida.conf

```

### Testing

You can test the server using a web browser by visiting the following built-in test routes:

* **Main Tester:** `http://localhost:8080/`
* **CGI Tester:** `http://localhost:8080/cgi`
* **Cookies Tester:** `http://localhost:8080/cookies`

You can also send automated or manual HTTP requests using `curl`:

```bash
curl -i http://localhost:8080/

To clean up object files and binary outputs:

```bash
make fclean

```

## Resources

### References

* **RFC 1945:** Hypertext Transfer Protocol — HTTP/1.0
* **Beej's Guide to Network Programming:** Fundamentals of Sockets and Network I/O
* **Nginx Official Documentation:** Location Matching, Root, and Alias Directive Behavior
* **MDN Web Docs:** HTTP Overview, Methods, and Status Codes

#### Articles & Tutorials
* **Medium:** *Understanding Non-Blocking I/O and Sockets in C++*
* **Dev.to:** *Building a Simple HTTP Server from Scratch in C++*
* **Medium:** *A Beginner's Guide to I/O Multiplexing (select, poll, epoll)*

### AI Usage

Artificial Intelligence (LLMs) was used during the project for:

* **Debugging & Edge Case Resolution:** Troubleshooting path resolution logic (location prefix handling, root concatenation, trailing slashes) and URI normalizations.
* **RFC Clarification:** Clarifying HTTP status code behavior, CGI environment variable requirements, and non-blocking socket handling under corner cases.
* **Code Refactoring:** Reviewing C++98 compliance and helping optimize non-blocking read/write state machines.