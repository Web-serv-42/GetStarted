#include "HTTP/Response/Response.hpp"

const std::string& Response::getFilePath() const {
    return this->_filePath;
}

// void Response::generateErrorResponse(int statusCode, const LocationConfig& config)
void Response::generateErrorResponse(HttpStatusCode statusCode)
{
    // (void)config;

    this->buildStatusLine(statusCode);

    this->_body = "<html><body><h1>Error " + this->_statusLine.substr(9) + "</h1></body></html>";

    std::stringstream ss;
    ss << this->_body.length();

    this->_headers = "Content-Type: text/html\r\n";
    this->_headers += "Content-Length: " + ss.str() + "\r\n";
}

Response::Response(){}

Response::~Response(){}

void    Response::Init(Routing routing, Request request)
{
    this->m_Request = request;
    this->m_Routing = routing;
}

HttpStatusCode Response::Run()
{
    // Optional for safety
    if (this->m_Request.GetErrorCode() != NORMAL) {
        return (this->m_Request.GetErrorCode());
    }

    HttpMethod method = this->m_Request.GetMethod();

    if (this->m_Routing.location != NULL && !this->m_Routing.location->allow_methods.empty())
    {
        std::string methodStr;
        if (method == HTTP_GET) methodStr = "GET";
        else if (method == HTTP_POST) methodStr = "POST";
        else if (method == HTTP_DELETE) methodStr = "DELETE";

        bool isAllowed = false;
        for (size_t i = 0; i < this->m_Routing.location->allow_methods.size(); ++i)
        {
            if (this->m_Routing.location->allow_methods[i] == methodStr)
            {
                isAllowed = true;
                break;
            }
        }

        if (!isAllowed)
        {
            return (HTTP_METHOD_NOT_ALLOWED);
        }
    }

    if (method == HTTP_GET) {
        return this->handleGet();
    } 
    else if (method == HTTP_POST) {
        return this->handlePost();
    } 
    else if (method == HTTP_DELETE) {
        return this->handleDelete();
    } 
    else {
        return (HTTP_METHOD_NOT_ALLOWED);
    }

    return (NORMAL);
}

std::string getContentTypeFromPath(const std::string& path)
{
    size_t dotPos = path.find_last_of(".");
    if (dotPos == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string ext = path.substr(dotPos);

    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "text/javascript";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".txt") return "text/plain";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".pdf") return "application/pdf";

    return "application/octet-stream";
}

// std::string Response::getRawResponse() const
// {
//     std::map<std::string, std::string>  cookies = this->m_Request.GetOutboundCookie();
//     if (!cookies.empty())
//     {
//         for (std::map<std::string, std::string>::const_iterator  it = cookies.begin(); it != cookies.end(); ++it)
//         {
//             this->_headers += "Set-Cookie: " + std::string(it->first) + "=" + std::string(it->second) + "\r\n";
//         }
//     }
//     std::string fullResponse = _statusLine + _headers + "\r\n" + _body;
//     return fullResponse;
// }

std::string Response::getRawResponse() const
{
    std::string headers = this->_headers;

    std::map<std::string, std::string>  cookies = this->m_Request.GetOutboundCookie();
    // Assuming m_OutboundCookies is a member map in Response (or retrieved via m_Request)
    if (!cookies.empty())
    {
        for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); 
             it != cookies.end(); 
             ++it)
        {
            headers += "Set-Cookie: " + it->first + "=" + it->second + "\r\n";
        }
    }

    return (this->_statusLine + headers + "\r\n" + this->_body);
}

void Response::buildStatusLine(HttpStatusCode statusCode)
{
    std::string reasonPhrase;
    
    reasonPhrase = GetHttpStatusReason(statusCode);

    std::stringstream ss;
    ss << statusCode;
    std::string codeStr = ss.str();

    this->_statusLine = "HTTP/1.0 " + codeStr + " " + reasonPhrase + "\r\n";
}

// GET ----------------------------------------------------------

std::string Response::generateAutoindex(const std::string& dirPath, const std::string& uri)
{
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        return "";
    }

    std::string html = "<html><head><title>Index of " + uri + "</title></head>";
    html += "<body><h1>Index of " + uri + "</h1><hr><pre>";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        
        if (name == ".") continue;

        html += "<a href=\"" + uri + (uri[uri.length() - 1] == '/' ? "" : "/") + name + "\">" + name + "</a>\n";
    }
    
    closedir(dir);
    html += "</pre><hr></body></html>";
    return html;
}

HttpStatusCode Response::handleGet()
{
    if (this->m_Routing.location != NULL && this->m_Routing.location->return_directive.first != NORMAL)
    {
        HttpStatusCode statusCode = this->m_Routing.location->return_directive.first;
        std::string redirectUrl = this->m_Routing.location->return_directive.second;

        if (redirectUrl.size() >= 2 && 
            ((redirectUrl[0] == '"' && redirectUrl[redirectUrl.size() - 1] == '"') || 
                (redirectUrl[0] == '\'' && redirectUrl[redirectUrl.size() - 1] == '\''))) 
        {
            redirectUrl = redirectUrl.substr(1, redirectUrl.size() - 2);
        }

        this->buildStatusLine(statusCode);
        this->_headers = "Location: " + redirectUrl + "\r\n";
        this->_headers += "Content-Length: 0\r\n";
        
        return (NORMAL); // <--- FIXED: Stop execution after redirect
    }

    std::string fullPath = this->m_Routing.filePath;
    std::string currentUri = this->m_Request.GetPath();

    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) != 0) {
        return (HTTP_NOT_FOUND);
    }

    if (S_ISDIR(fileStat.st_mode)) 
    {
        std::string indexFile = (this->m_Routing.location && !this->m_Routing.location->index.empty()) ? this->m_Routing.location->index : "index.html";
        
        if (indexFile.size() >= 2 && 
            ((indexFile[0] == '"' && indexFile[indexFile.size() - 1] == '"') || 
                (indexFile[0] == '\'' && indexFile[indexFile.size() - 1] == '\''))) 
        {
            indexFile = indexFile.substr(1, indexFile.size() - 2);
        }

        std::string indexPath = fullPath + (fullPath[fullPath.length() - 1] == '/' ? "" : "/") + indexFile;
        
        struct stat indexStat;
        if (stat(indexPath.c_str(), &indexStat) == 0 && S_ISREG(indexStat.st_mode)) {
            fullPath = indexPath;
        }
        else {
            if (this->m_Routing.location && this->m_Routing.location->autoindex == true) {
                this->_body = this->generateAutoindex(fullPath, currentUri);
                if (this->_body.empty()) {
                    return (HTTP_INTERNAL_SERVER_ERROR);
                }
                this->buildStatusLine(HTTP_OK);
                std::stringstream ss;
                ss << this->_body.length();
                this->_headers = "Content-Type: text/html\r\n";
                this->_headers += "Content-Length: " + ss.str() + "\r\n";
                return (NORMAL); // <--- FIXED: Autoindex returns HTML in memory successfully
            } else {
                return (HTTP_FORBIDDEN);
            }
        }
    }

    if (access(fullPath.c_str(), R_OK) != 0) {
        return (HTTP_FORBIDDEN);
    }

    // <--- BIG ARCHITECTURE FIX: Do NOT read the whole file into RAM! --->
    struct stat fileStat2;
    if (stat(fullPath.c_str(), &fileStat2) != 0) {
        return (HTTP_INTERNAL_SERVER_ERROR);
    }

    this->_filePath = fullPath; // Store path for Client to read chunks
    this->_body = "";           // Keep body empty

    this->buildStatusLine(HTTP_OK);
    std::stringstream ss;
    ss << fileStat2.st_size; // Exact file size from stat

    std::string contentType = getContentTypeFromPath(fullPath);
    this->_headers = "Content-Type: " + contentType + "\r\n";
    this->_headers += "Content-Length: " + ss.str() + "\r\n";
    
    return (NORMAL); // <--- FIXED: End of handleGet
}

// Delete ----------------------------------------------------------

HttpStatusCode Response::handleDelete()
{
    std::string fullPath = this->m_Routing.filePath;

    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) != 0) {
        return (HTTP_NOT_FOUND);
    }

    if (S_ISDIR(fileStat.st_mode)) {
        return (HTTP_FORBIDDEN);
    }

    if (access(fullPath.c_str(), W_OK) != 0) {
        return (HTTP_FORBIDDEN);
    }

    if (unlink(fullPath.c_str()) != 0) {
        return (HTTP_INTERNAL_SERVER_ERROR);
    }

    this->buildStatusLine(HTTP_NO_CONTENT);
    this->_headers = "Content-Length: 0\r\n";
    this->_body = "";
    return (NORMAL); // <--- FIXED: Added missing return statement
}

// Post ----------------------------------------------------------

HttpStatusCode Response::handlePost()
{
    size_t limitInMB = m_Routing.location->client_max_body_size;
    size_t limitInBytes = limitInMB * 1024 * 1024;

    if (limitInMB > 0 && m_Request.GetBodyReceived() > limitInBytes)
    {
        return (HTTP_PAYLOAD_TOO_LARGE);
    }

    std::string uploadDir = ".";
    if (this->m_Routing.location) {
        if (!this->m_Routing.location->upload_file.empty())
            uploadDir = this->m_Routing.location->upload_file;
        else if (!this->m_Routing.location->root.empty())
            uploadDir = this->m_Routing.location->root;
    }

    std::string createdResourceUrl = "";

    if (this->m_Request.IsMultipart()) 
    {
        const std::vector<MultipartPart>& parts = this->m_Request.GetParts();
        
        for (size_t i = 0; i < parts.size(); ++i) {
            if (parts[i].fileName.empty()) continue;

            std::string destPath = uploadDir + "/" + parts[i].fileName;
            
            if (std::rename(parts[i].tmpFilePath.c_str(), destPath.c_str()) != 0) {
                std::cout << "\033[1;31m[POST] Failed to rename: " << parts[i].fileName << "\033[0m\n";
                return (HTTP_INTERNAL_SERVER_ERROR);
            }

            // Track the client-facing HTTP URL path of the first created file
            if (createdResourceUrl.empty()) {
                std::string reqPath = this->m_Request.GetPath();
                if (!reqPath.empty() && reqPath[reqPath.length() - 1] == '/')
                    createdResourceUrl = reqPath + parts[i].fileName;
                else
                    createdResourceUrl = reqPath + "/" + parts[i].fileName;
            }
        }

        this->buildStatusLine(HTTP_CREATED);
        this->_body = "";
    }
    else
    {
        // Non-multipart POST request
        this->buildStatusLine(HTTP_OK);
        this->_body = "";
    }

    this->_headers = "";

    if (!createdResourceUrl.empty()) {
        this->_headers += "Location: " + createdResourceUrl + "\r\n";
    }

    std::stringstream ss;
    ss << this->_body.length();
    
    if (!this->_body.empty()) {
        this->_headers += "Content-Type: text/html\r\n";
    }
    
    this->_headers += "Content-Length: " + ss.str() + "\r\n";
    
    return (NORMAL);
}

// Handle Error--------------------------------

void Response::handleError(HttpStatusCode statusCode)
{
    std::string errorPageFile = "";

    if (this->m_Routing.location != NULL) {
        std::map<int, std::string>::const_iterator it = this->m_Routing.location->error_pages.find(statusCode);
        if (it != this->m_Routing.location->error_pages.end()) {
            errorPageFile = it->second;
        }
    }
    
    if (errorPageFile.empty() && this->m_Routing.server != NULL) {
        std::map<int, std::string>::const_iterator it = this->m_Routing.server->error_pages.find(statusCode);
        if (it != this->m_Routing.server->error_pages.end()) {
            errorPageFile = it->second;
        }
    }

    if (!errorPageFile.empty())
    {
        std::string rootDir = ".";
        if (this->m_Routing.location && !this->m_Routing.location->root.empty()) {
            rootDir = this->m_Routing.location->root;
        } else if (this->m_Routing.server && !this->m_Routing.server->root.empty()) {
            rootDir = this->m_Routing.server->root;
        }

        std::string fullPath = rootDir + (errorPageFile[0] == '/' ? "" : "/") + errorPageFile;

        // <--- BIG ARCHITECTURE FIX: Do NOT read the custom error page into RAM! --->
        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0 && access(fullPath.c_str(), R_OK) == 0) 
        {
            this->_filePath = fullPath; // Store path for Client to read chunks
            this->_body = "";           // Keep body empty

            this->buildStatusLine(statusCode);

            std::stringstream ss;
            ss << fileStat.st_size; // Exact file size from stat

            std::string contentType = getContentTypeFromPath(fullPath); 
            this->_headers = "Content-Type: " + contentType + "\r\n";
            this->_headers += "Content-Length: " + ss.str() + "\r\n";
            
            return;
        }
        else {
            std::cout << "\033[1;31m[ERROR] Custom error page not found on disk: " << fullPath << "\033[0m" << std::endl;
        }
    }

    // Fallback: Generate a small string in memory if no file exists
    this->generateErrorResponse(statusCode);
}
