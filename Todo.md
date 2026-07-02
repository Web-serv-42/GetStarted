## Webserv
- [ ] Efficient server/client management
- [ ] Default + Error pages
- [ ] Bad response + Status code

## Config File
- [ ] Need implementaion

## HttpRequest
- [ ] Need implementaion

## HttpResponse
- [ ] Need implementaion


- [ ] You need to manage Correct response, bad response correctly both from normal or CGI request [edge case] `www/` folder

## CGI
### Chunked transfer encoding
- [ ] For chunked requests (Temp File Method) : 
	- [ ] HTTP Parser strips out the chunk hex codes, writes only the pure data to the Temp File
	- [ ] Set the maximum allowed size for client request bodies.
	- [ ] The CGI should be run in the correct directory for relative path file access.



# 🛠 Task 1: The Event Manager (Member 1 - You)

    [ ] Implement the Polling class (wrapper for epoll).

    [ ] Implement the Client state machine (READ_REQ, WAIT_CGI, SEND_RES).

    [ ] Implement the CGI Executor (The fork/exec logic and pipe collection).

    [ ] The Pipe-to-File Streamer: Logic that reads from a CGI pipe and writes to a file.

# 📝 Task 2: The Data Translator (Member 2)

    [ ] Implement the Request class that identifies headers and handles "Un-chunking" (if you decide to support it later).

    [ ] Implement a BodyHandler that manages the writing/reading of the Body Tmp_file.

# 🗺 Task 3: The Decision Maker (Member 3)

    [ ] Implement the ConfigParser.

    [ ] Implement the Router that takes a Request and a Config and returns a Location object.

    [ ] Implement the ResponseBuilder that generates the final HTTP/1.1 200 OK... string.

# (ablabib) Task: 
    ConfigResolver::GetPorts()
    ConfigResolver::GetServersForPort() (needed for virtual hosts)
    ConfigResolver::FindServer(host, port) (needed after HTTP parsing)
    ConfigResolver::FindLocation(uri) (needed for routing)
    ConfigResolver::GetRoot(), GetIndex(), MethodAllowed(), etc. (needed when serving requests)