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

Response::Response(Routing routing, Request request) : m_Routing(routing), m_Request(request)
{

}

Response::~Response(){}

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

std::string Response::getRawResponse() const
{
    std::string fullResponse = _statusLine + _headers + "\r\n" + _body;
    return fullResponse;
}

void Response::buildStatusLine(HttpStatusCode statusCode)
{
    std::string reasonPhrase;
    
    reasonPhrase = GetHttpStatusReason(statusCode);

    std::stringstream ss;
    ss << statusCode;
    std::string codeStr = ss.str();

    this->_statusLine = "HTTP/1.1 " + codeStr + " " + reasonPhrase + "\r\n";
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



// Helper function to parse and save multipart form data
HttpStatusCode Response::handleMultipartUpload(const std::string& uploadDir, const std::string& contentType)
{
    // Extract the boundary string
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) return HTTP_BAD_REQUEST;
    
    std::string boundary = "--" + contentType.substr(boundaryPos + 9);
    std::string endBoundary = boundary + "--";

    // Open the raw body file that Webserv saved
    std::ifstream bodyFile(this->m_Request.GetBodyFilePath().c_str(), std::ios::binary);
    if (!bodyFile.is_open()) return HTTP_INTERNAL_SERVER_ERROR;

    // Read the entire file into a string (Note: for 1GB+ multipart, this should be chunked, 
    // but this works perfectly for standard browser uploads)
    std::stringstream buffer;
    buffer << bodyFile.rdbuf();
    std::string bodyContent = buffer.str();
    bodyFile.close();

    size_t pos = 0;
    while ((pos = bodyContent.find(boundary, pos)) != std::string::npos)
    {
        // Break if we hit the end boundary
        if (bodyContent.substr(pos, endBoundary.length()) == endBoundary) {
            break;
        }

        pos += boundary.length() + 2; // Move past boundary and \r\n
        size_t headerEnd = bodyContent.find("\r\n\r\n", pos);
        if (headerEnd == std::string::npos) break;

        std::string headers = bodyContent.substr(pos, headerEnd - pos);
        
        // Extract filename
        size_t filenamePos = headers.find("filename=\"");
        if (filenamePos != std::string::npos)
        {
            filenamePos += 10; // length of 'filename="'
            size_t filenameEnd = headers.find("\"", filenamePos);
            std::string extractedFilename = headers.substr(filenamePos, filenameEnd - filenamePos);

            // Find where this file's data ends (the next boundary)
            size_t dataStart = headerEnd + 4; // Move past \r\n\r\n
            size_t dataEnd = bodyContent.find("\r\n" + boundary, dataStart);
            if (dataEnd == std::string::npos) dataEnd = bodyContent.length();

            // Save the extracted file
            if (!extractedFilename.empty())
            {
                std::string destPath = uploadDir + "/" + extractedFilename;
                std::ofstream outFile(destPath.c_str(), std::ios::binary);
                if (outFile.is_open()) {
                    outFile.write(bodyContent.c_str() + dataStart, dataEnd - dataStart);
                    outFile.close();
                    std::cout << "\033[1;32m[POST MULTIPART] Saved file: " << destPath << "\033[0m\n";
                }
            }
        }
        pos = headerEnd + 4; // Move forward to parse the next part
    }
    return NORMAL;
}

HttpStatusCode Response::handlePost()
{
    size_t limitSize = 0;
    if (this->m_Routing.location && this->m_Routing.location->client_max_body_size > 0) {
        limitSize = this->m_Routing.location->client_max_body_size;
    } else {
        limitSize = 20;
    }

    if (limitSize < 1024) {
        limitSize = limitSize * 1024 * 1024;
    }

    std::cout << "\033[1;36m[DEBUG POST] limitSize: " << limitSize 
                << " | GetBodyReceived(): " << this->m_Request.GetBodyReceived() << "\033[0m" << std::endl;

    if (this->m_Request.GetBodyReceived() > limitSize) {
        return (HTTP_PAYLOAD_TOO_LARGE);
    }

    std::string uploadDir;
    if (this->m_Routing.location && !this->m_Routing.location->upload_file.empty()) {
        uploadDir = this->m_Routing.location->upload_file;
    } else if (this->m_Routing.location && !this->m_Routing.location->root.empty()) {
        uploadDir = this->m_Routing.location->root;
    } else {
        uploadDir = ".";
    }

    // --- CHECK CONTENT TYPE FOR BROWSER FORMS ---
    std::string contentType = this->m_Request.GetHeader("content-type");
    
    if (contentType.find("multipart/form-data") != std::string::npos) 
    {
        std::cout << "\033[1;35m[POST] Multipart upload detected! Parsing boundary...\033[0m\n";
        HttpStatusCode status = this->handleMultipartUpload(uploadDir, contentType);
        if (status != NORMAL) {
            return status;
        }
    }
    else 
    {
        // --- NORMAL RAW UPLOAD (curl --data-binary) ---
        std::cout << "\033[1;36m[POST] Raw file upload detected!\033[0m\n";
        
        std::string uri = this->m_Request.GetPath();
        size_t lastSlash = uri.find_last_of('/');
        std::string fileName = "";

        if (lastSlash != std::string::npos && lastSlash + 1 < uri.size()) {
            fileName = uri.substr(lastSlash + 1);
        }

        std::string locationPath = "";
        if (this->m_Routing.location) {
            locationPath = this->m_Routing.location->path;
            if (locationPath.length() > 0 && locationPath[0] == '/') {
                locationPath = locationPath.substr(1);
            }
        }

        // Use timestamp if no filename was provided in the URI
        if (fileName.empty() || fileName == locationPath) {
            std::stringstream timeStream;
            timeStream << "file_" << time(NULL) << ".bin";
            fileName = timeStream.str();
        }

        std::string destFile = uploadDir + "/" + fileName;

        std::cout << "\033[1;33m[POST DEBUG] Destination file path: " << destFile << "\033[0m" << std::endl;
        
        std::ifstream src(this->m_Request.GetBodyFilePath().c_str(), std::ios::binary);
        if (!src.is_open()) {
            std::cout << "\033[1;31m[POST DEBUG] Failed to open body file!\033[0m" << std::endl;
            return (HTTP_INTERNAL_SERVER_ERROR);
        }

        std::ofstream dst(destFile.c_str(), std::ios::binary);
        if (!dst.is_open()) {
            std::cout << "\033[1;31m[POST DEBUG] Failed to write to: " << destFile << "\033[0m" << std::endl;
            src.close();
            return (HTTP_INTERNAL_SERVER_ERROR);
        }

        dst << src.rdbuf();
        src.close();
        dst.close();
    }

    // --- SUCCESS RESPONSE ---
    this->buildStatusLine(HTTP_CREATED);
    this->_body = "<html><body><h1>201 Created: File Uploaded Successfully!</h1></body></html>";
    
    std::stringstream ss;
    ss << this->_body.length();
    this->_headers = "Content-Type: text/html\r\n";
    this->_headers += "Content-Length: " + ss.str() + "\r\n";
    
    return (NORMAL);
}

// HttpStatusCode Response::handlePost()
// {
//     size_t limitSize = 0;
//     if (this->m_Routing.location && this->m_Routing.location->client_max_body_size > 0) {
//         limitSize = this->m_Routing.location->client_max_body_size;
//     } else {
//         limitSize = 20;
//     }

//     if (limitSize < 1024) {
//         limitSize = limitSize * 1024 * 1024;
//     }

//     std::cout << "\033[1;36m[DEBUG POST] limitSize: " << limitSize 
//                 << " | GetBodyReceived(): " << this->m_Request.GetBodyReceived() << "\033[0m" << std::endl;

//     if (this->m_Request.GetBodyReceived() > limitSize) {
//         return (HTTP_PAYLOAD_TOO_LARGE);
//     }

//     std::string uploadDir;
//     if (this->m_Routing.location && !this->m_Routing.location->upload_file.empty()) {
//         uploadDir = this->m_Routing.location->upload_file;
//     } else if (this->m_Routing.location && !this->m_Routing.location->root.empty()) {
//         uploadDir = this->m_Routing.location->root;
//     } else {
//         uploadDir = ".";
//     }

//     std::string uri = this->m_Request.GetPath();
//     size_t lastSlash = uri.find_last_of('/');
//     std::string fileName = "uploaded_file.txt" ;// need dynamic file and extension attrubutes;
//     if (lastSlash != std::string::npos && lastSlash + 1 < uri.size()) {
//         fileName = uri.substr(lastSlash + 1);
//     }

//     std::string destFile = uploadDir + "/" + fileName;

//     std::cout << "\033[1;33m[POST DEBUG] Destination file path: " << destFile << "\033[0m" << std::endl;
    
//     std::ifstream src(this->m_Request.GetBodyFilePath().c_str(), std::ios::binary);
//     if (!src.is_open()) {
//         std::cout << "\033[1;31m[POST DEBUG] Failed to open this->m_Request Body File!\033[0m" << std::endl;
//         return (HTTP_INTERNAL_SERVER_ERROR);
//     }

//     std::ofstream dst(destFile.c_str(), std::ios::binary);
//     if (!dst.is_open()) {
//         std::cout << "\033[1;31m[POST DEBUG] Failed to write to: " << destFile << ". Check folder existence!\033[0m" << std::endl;
//         src.close();
//         return (HTTP_INTERNAL_SERVER_ERROR);
//     }

//     dst << src.rdbuf();
//     src.close();
//     dst.close();

//     this->buildStatusLine(HTTP_CREATED);
//     this->_body = "<html><body><h1>201 Created: File Uploaded Successfully!</h1></body></html>";
    
//     std::stringstream ss;
//     ss << this->_body.length();
//     this->_headers = "Content-Type: text/html\r\n";
//     this->_headers += "Content-Length: " + ss.str() + "\r\n";
    
//     return (NORMAL);
// }

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
