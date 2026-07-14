#include "HTTP/Response/Response.hpp"
#include "Client/Client.hpp"

// void Response::generateErrorResponse(int statusCode, const LocationConfig& config)
void Response::generateErrorResponse(int statusCode)
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

Response::Response(Client& client)
{
    if (client.GetRequest().GetErrorCode() != 0) {
        this->generateErrorResponse(client.GetRequest().GetErrorCode());
        return;
    }

    HttpMethod method = client.GetRequest().GetMethod();
    const Routing& routing = client.GetRouting();

    if (routing.location != NULL && !routing.location->allow_methods.empty())
    {
        std::string methodStr;
        if (method == HTTP_GET) methodStr = "GET";
        else if (method == HTTP_POST) methodStr = "POST";
        else if (method == HTTP_DELETE) methodStr = "DELETE";

        bool isAllowed = false;
        for (size_t i = 0; i < routing.location->allow_methods.size(); ++i)
        {
            if (routing.location->allow_methods[i] == methodStr)
            {
                isAllowed = true;
                break;
            }
        }

        if (!isAllowed)
        {
            this->generateErrorResponse(405);
            return;
        }
    }

    if (method == HTTP_GET) {
        this->handleGet(client);
    } 
    else if (method == HTTP_POST) {
        this->handlePost(client);
    } 
    else if (method == HTTP_DELETE) {
        this->handleDelete(client);
    } 
    else {
        this->generateErrorResponse(405);
    }
}


Response::~Response(){}

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

void Response::buildStatusLine(int statusCode)
{
    std::string reasonPhrase;

    switch (statusCode) {
        case 200: reasonPhrase = "OK"; break;
        case 201: reasonPhrase = "Created"; break;
        case 204: reasonPhrase = "No Content"; break;
        case 301: reasonPhrase = "Moved Permanently"; break;
        case 400: reasonPhrase = "Bad Request"; break;
        case 403: reasonPhrase = "Forbidden"; break;
        case 404: reasonPhrase = "Not Found"; break;
        case 405: reasonPhrase = "Method Not Allowed"; break;
        case 413: reasonPhrase = "Payload Too Large"; break;
        case 500: reasonPhrase = "Internal Server Error"; break;
        case 505: reasonPhrase = "HTTP Version Not Supported"; break;
        default:  reasonPhrase = "Unknown Status"; break;
    }

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

void Response::handleGet(Client& client)
{
    const Routing& routing = client.GetRouting();
    const Request& request = client.GetRequest();

    if (routing.location != NULL && routing.location->return_directive.first != 0)
    {
        int statusCode = routing.location->return_directive.first;
        std::string redirectUrl = routing.location->return_directive.second;

        if (redirectUrl.size() >= 2 && 
            ((redirectUrl[0] == '"' && redirectUrl[redirectUrl.size() - 1] == '"') || 
             (redirectUrl[0] == '\'' && redirectUrl[redirectUrl.size() - 1] == '\''))) 
        {
            redirectUrl = redirectUrl.substr(1, redirectUrl.size() - 2);
        }

        this->buildStatusLine(statusCode);
        this->_headers = "Location: " + redirectUrl + "\r\n";
        this->_headers += "Content-Length: 0\r\n";
        return;
    }

    std::string fullPath = routing.filePath;
    std::string currentUri = request.GetPath();

    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) != 0) {
        this->generateErrorResponse(404);
        return;
    }

    if (S_ISDIR(fileStat.st_mode)) 
    {
        std::string indexFile = (routing.location && !routing.location->index.empty()) ? routing.location->index : "index.html";
        
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
            if (routing.location && routing.location->autoindex == true) {
                this->_body = this->generateAutoindex(fullPath, currentUri);
                if (this->_body.empty()) {
                    this->generateErrorResponse(500);
                    return;
                }
                this->buildStatusLine(200);
                std::stringstream ss;
                ss << this->_body.length();
                this->_headers = "Content-Type: text/html\r\n";
                this->_headers += "Content-Length: " + ss.str() + "\r\n";
                return;
            } else {
                this->generateErrorResponse(403);
                return;
            }
        }
    }

    if (access(fullPath.c_str(), R_OK) != 0) {
        this->generateErrorResponse(403);
        return;
    }

    std::ifstream file(fullPath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        this->generateErrorResponse(500);
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    this->_body = content;
    file.close();

    this->buildStatusLine(200);
    std::stringstream ss;
    ss << this->_body.length();

    std::string contentType = getContentTypeFromPath(fullPath);
    this->_headers = "Content-Type: " + contentType + "\r\n";
    this->_headers += "Content-Length: " + ss.str() + "\r\n";
}

// Delete ----------------------------------------------------------

void Response::handleDelete(Client& client)
{
    const Routing& routing = client.GetRouting();
    std::string fullPath = routing.filePath;

    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) != 0) {
        this->generateErrorResponse(404);
        return;
    }

    if (S_ISDIR(fileStat.st_mode)) {
        this->generateErrorResponse(403);
        return;
    }

    if (access(fullPath.c_str(), W_OK) != 0) {
        this->generateErrorResponse(403);
        return;
    }

    if (unlink(fullPath.c_str()) != 0) {
        this->generateErrorResponse(500);
        return;
    }

    this->buildStatusLine(204);
    this->_headers = "Content-Length: 0\r\n";
    this->_body = "";
}


// Post ----------------------------------------------------------

void Response::handlePost(Client& client)
{
    const Routing& routing = client.GetRouting();
    const Request& request = client.GetRequest();

    size_t limitSize = 0;
    if (routing.location && routing.location->client_max_body_size > 0) {
        limitSize = routing.location->client_max_body_size;
    } else {
        limitSize = 20;
    }

    if (limitSize < 1024) {
        limitSize = limitSize * 1024 * 1024;
    }

    std::cout << "\033[1;36m[DEBUG POST] limitSize: " << limitSize 
              << " | GetBodyReceived(): " << request.GetBodyReceived() << "\033[0m" << std::endl;

    if (request.GetBodyReceived() > limitSize) {
        this->generateErrorResponse(413);
        return;
    }

    std::string uploadDir;
    if (routing.location && !routing.location->upload_file.empty()) {
        uploadDir = routing.location->upload_file;
    } else if (routing.location && !routing.location->root.empty()) {
        uploadDir = routing.location->root;
    } else {
        uploadDir = ".";
    }

    std::string uri = request.GetPath();
    size_t lastSlash = uri.find_last_of('/');
    std::string fileName = "uploaded_file.txt";
    if (lastSlash != std::string::npos && lastSlash + 1 < uri.size()) {
        fileName = uri.substr(lastSlash + 1);
    }

    std::string destFile = uploadDir + "/" + fileName;

    std::cout << "\033[1;33m[POST DEBUG] Destination file path: " << destFile << "\033[0m" << std::endl;
    
    std::ifstream src(request.GetBodyFilePath().c_str(), std::ios::binary);
    if (!src.is_open()) {
        std::cout << "\033[1;31m[POST DEBUG] Failed to open Request Body File!\033[0m" << std::endl;
        this->generateErrorResponse(500);
        return;
    }

    std::ofstream dst(destFile.c_str(), std::ios::binary);
    if (!dst.is_open()) {
        std::cout << "\033[1;31m[POST DEBUG] Failed to write to: " << destFile << ". Check folder existence!\033[0m" << std::endl;
        src.close();
        this->generateErrorResponse(500);
        return;
    }

    dst << src.rdbuf();
    src.close();
    dst.close();

    this->buildStatusLine(201);
    this->_body = "<html><body><h1>201 Created: File Uploaded Successfully!</h1></body></html>";
    
    std::stringstream ss;
    ss << this->_body.length();
    this->_headers = "Content-Type: text/html\r\n";
    this->_headers += "Content-Length: " + ss.str() + "\r\n";
}