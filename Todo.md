## Webserv
- [~] Efficient server/client management
- [ ] Default + Error pages
- [ ] Bad response + Status code
- [~] Status Code
- [x] CGI needs Parametres from Request object
- [x] Client Timeouts
- [x] Fd leaks body
- [x] py scipt hang (check access to file or path exist in CGI)

- [ ] Chuncked request is mandatory transfer-encoding header, chunks format:
    - length\r\n
    - body-chunk \r\n
    - 0\r\n
    - \r\n

- [ ] Since ur (ablabib) filling the body upload to a tmp file (uploads direclty):
    - [ ] Single File Upload (Direct Binary)
    - [ ] Multipart Form-Data (Multiple Files)

- [x] 400 Bad Request
- [ ] 403 Forbidden
- [ ] 404 Not Found
- [ ] 405 Method Not Allowed
- [x] 408 Request Timeout
- [ ] 411 Length Required
- [ ] 413 Payload Too Large (Body max size check)
- [ ] 415 Unsupported Media Type
- [x] 500 Internal Server Error
- [ ] 501 Not Implemented
- [x] 502 Bad Gateway
- [x] 504 Gateway Timeout


## ====================== How to use 42 intra test ======================
- We already have the `tester` executable in the root directory
- Also the `cgi-tester` `in cgi-bin/` directory
- We need two terminals:
    * launch tester: `./tester http://127.0.0.1:8080`
    * launch webserv: `./webserv config/tester.config`
- The teser give instruction to setup folders `YoupiBanane`, and config file `tester.config` [We already have this]
- press enter, enter, ... to begin testing [We need response to pas tests `wa hamzaaaaaaaaaaaaaaaaa`]


## ================================ HttpResponse (Hamza) ================================
- [ ] Need implementaion

- [ ] You need to manage Correct response, bad response correctly both from normal or CGI request [edge case] `root` folder

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
        return (0);;
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
            return (0);;
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
    return (0);;
}
``` 
- [ ] bad name for upload files [space parsing]
- [ ] At vuild time => create /tmp folder, fclean => delete /tmp