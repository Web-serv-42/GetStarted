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

 [ ] Implement the ResponseBuilder that generates the final HTTP/1.1 200 OK... string.

## Prepare Write Buffer from Response builder object
```cpp
int Client::PrepareWriteBuffer()
{
    struct stat         fileInfo;
    std::stringstream   headerStream;

    // 0. Handle quick in-memory responses (Errors or small responses)
    if ((this->m_State == STATE_SENDING_ERROR_RESPONSE || this->m_State == STATE_SENDING_FULL_RESPONSE)
        && !this->m_WriteBuffer.empty())
    {
        // Since the response here is small the response builder already store it in-memory, a std::string is enougth
        this->m_WriteBuffer = this->m_Response->m_FullResponse;
        this->m_State = STATE_RESPONSE_SENT;
        return (200);
    }

    // Dynamically identify our data source file target
    if (this->m_State == STATE_SENDING_HEADERS)
    {
        if (this->m_CGI) // If it was a CGI transaction
            this->m_FileContentPath = this->m_CGI->GetTmpOutputFile();
        else            // If it was a static request handled by Member 2
            this->m_FileContentPath = this->m_Response->m_ResponseFilePath; 

        if (stat(this->m_FileContentPath.c_str(), &fileInfo) != 0)
        {
            ERROR_LOG("Could not find source file to measure size");
            return (500);
        }

        // Before sending headers we check if a body exist and open the bodyFile [static file, cgi output file]
        if (this->m_Response->m_HasBody)
        {
            // Open the descriptor for streaming phase
            this->m_ContentFileFd = open(this->m_FileContentPath.c_str(), O_RDONLY);
            if (this->m_ContentFileFd == -1)
            {
                ERROR_LOG("Could not open source file");
                return (500);
            }
        }
        // 1. Let Member 2 pass down the pre-built headers, OR generate standard ones
        if (!this->m_CGI && !this->m_Response->m_HeadersBuffer.empty())
        {
            this->m_WriteBuffer = this->m_Response->m_HeadersBuffer;
            return (200);
        }
        else if (this->m_CGI)
        {
            // Case1: Extract Headers if they exists then return 200 [and set state to STATE_SENDING_BODY]
            // Case2: if Headers not exist just continue down and create headers from zero ...
        }
        // Simple fallback headers generation
        headerStream << "HTTP/1.1 200 OK\r\n"
                        << "Content-Type: text/html\r\n"
                        << "Content-Length: " << fileInfo.st_size << "\r\n"
                        << "\r\n";
        this->m_WriteBuffer = headerStream.str();
        this->m_State = STATE_SENDING_BODY;
    }

        // 2. Read body [static file/cgi output file] chunk-by-chunk BUFFER_SIZE=4096
    else if (this->m_State == STATE_SENDING_BODY)
    {
        if (this->m_WriteBuffer.empty())
        {
            int res = this->ReadFileContent();
            if (res == -1)
            {
                close(this->m_ContentFileFd);   
                return (500);
            }
            if (res == 0) 
            {
                close(this->m_ContentFileFd);
                this->m_State = STATE_RESPONSE_SENT;
            }
        }
    }
    return (200);
}
``` 