#include "Parsing/RequestParser.hpp"

// =========================================================================
// HELPER METHODS
// =========================================================================
std::string RequestParser::ToLowercase(const std::string& str) {
	std::string result = str;
	for (size_t i = 0; i < result.length(); ++i) {
		result[i] = std::tolower(result[i]);
	}
	return result;
}

// trim the request cuz i could be inforced
std::string RequestParser::Trim(const std::string& str) {
	size_t first = str.find_first_not_of(" \t"); // we dont \t cuz there is no tabs between but u never know
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t");
	return str.substr(first, (last - first + 1));
}
	
// =========================================================================
// PARSERS
// =========================================================================
void RequestParser::ParseRequestLine(Request& req, const std::string& line)
{
	std::istringstream iss(line);
	std::string methodStr, rawUri, version;
	std::string extra;
	if (!(iss >> methodStr >> rawUri >> version) || (iss >> extra))
	{
		req.SetErrorCode(HTTP_BAD_REQUEST);
		req.SetState(PARSE_ERROR);
		return;
	}

	// 1. Map Method
	if (methodStr == "GET") req.SetMethod(HTTP_GET);
	else if (methodStr == "POST") req.SetMethod(HTTP_POST);
	else if (methodStr == "DELETE") req.SetMethod(HTTP_DELETE);
	else {
		req.SetErrorCode(HTTP_BAD_REQUEST); // Bad Request
		req.SetState(PARSE_ERROR);
		return;
	}
	// I know now that the method is valid, let's store it into std::string m_MethodString
	req.SetMethodString(methodStr);
	// URI must start with / 
	if (rawUri.empty() || rawUri[0] != '/')
	{
		req.SetErrorCode(HTTP_BAD_REQUEST);
		req.SetState(PARSE_ERROR);
		return;
	}

	// 2. ignore what is comming after # edge case (Fragment identifier)
	size_t hashPos = rawUri.find('#');
	if (hashPos != std::string::npos) rawUri = rawUri.substr(0, hashPos);

	// 3. Split Path and Query String
	// std::string extractedPath;
	size_t questionPos = rawUri.find('?');
	if (questionPos != std::string::npos) {
		req.SetPath(rawUri.substr(0, questionPos));
		req.SetQuery(rawUri.substr(questionPos + 1));
	} else {
		req.SetPath(rawUri);
		req.SetQuery("");
	}
	
	// we need to extract the uri here meaning we remove the ".." if the user tried to go above root 
	// we check if the URI request is inside the root path 
	// std::string safePath = is_SafePath(extractedPath);
	// req.SetPath(safePath);

	req.SetVersion(version);
}

// std::string  RequestParser::is_SafePath(const std::string& URI)
// {


// }


void RequestParser::ParseHeader(Request& req, const std::string& line) {
	size_t colonPos = line.find(':');
	if (colonPos == std::string::npos) {
		req.SetErrorCode(HTTP_BAD_REQUEST);
		req.SetState(PARSE_ERROR);
		return;
	}

	// KeyName:[colon][space]Value\r\n | [KeyName: ]
	std::string key = ToLowercase(Trim(line.substr(0, colonPos)));
	std::string value = Trim(line.substr(colonPos + 1));

	if (key.empty())
	{
		req.SetErrorCode(HTTP_BAD_REQUEST);
		req.SetState(PARSE_ERROR);
		return;
	}

	// Reject duplicate Content-Length
	if (key == "content-length" &&
		!req.GetHeader("content-length").empty())
	{
		req.SetErrorCode(HTTP_BAD_REQUEST);
		req.SetState(PARSE_ERROR);
		return;
	}

	if (key == "cookie")
    {
        ParseCookies(req, value);
    }

	req.AddHeader(key, value);
}

void	RequestParser::ParseCookies(Request& req, const std::string& line) {
    size_t start = 0;

    while (start < line.length()) {
        // Find the next cookie pair separator
        size_t end = line.find(';', start);
        std::string pair = line.substr(start, (end == std::string::npos) ? std::string::npos : (end - start));

        // Trim leading space if present (browsers send: "cookie1=a; cookie2=b")
        size_t firstNotSpace = pair.find_first_not_of(" \t");
        if (firstNotSpace != std::string::npos) {
            pair = pair.substr(firstNotSpace);
        }

        // Split key and value by '='
        size_t equalSign = pair.find('=');
        if (equalSign != std::string::npos) {
            std::string key = pair.substr(0, equalSign);
            std::string value = pair.substr(equalSign + 1);
            
            if (!key.empty()) {
				req.AddCookie(key, value);
            }
        }

        if (end == std::string::npos)
            break;
        start = end + 1;
    }
}

// Helper function to extract the real filename from the Content-Disposition block
std::string RequestParser::ExtractExtensionFromHeaders(const std::string& headers)
{
    const std::string key = "filename=\"";

    size_t start = headers.find(key);
    if (start == std::string::npos)
        return "";

    start += key.length();

    size_t end = headers.find('"', start);
    if (end == std::string::npos)
        return "";

    std::string filename = headers.substr(start, end - start);

	std::string extension = ".txt";

    // Find the last '.'
    size_t dot = filename.rfind('.');

    if (dot != std::string::npos &&
        dot != 0 &&
        dot != filename.length() - 1)
    {
        extension = filename.substr(dot);
    }
	
    return extension;
}

bool RequestParser::ParseMultipartBody(Request& req, std::string& rawBuffer)
{
    std::string boundary = req.GetBoundary();
    std::string endBoundary = boundary + "--";

    // Update total received for Content-Length tracking
    req.AddBodyReceived(rawBuffer.length()); 

    while (!rawBuffer.empty())
    {
        size_t boundPos = rawBuffer.find(boundary);

        if (boundPos != std::string::npos) 
        {
            // 1. A boundary was found! Write any preceding data to the CURRENT open file
            if (boundPos > 0 && req.HasOpenMultipartPart()) {
                size_t writeLen = boundPos;
                
                // FIX: Strip the preceding \r\n that belongs to the multipart protocol, not the file!
                if (writeLen >= 2 && rawBuffer[writeLen - 2] == '\r' && rawBuffer[writeLen - 1] == '\n') {
                    writeLen -= 2;
                }
                
                req.WriteToCurrentMultipartPart(rawBuffer.substr(0, writeLen));
            }
            req.CloseCurrentMultipartPart();

            // 2. Check if it's the final ending boundary
            if (rawBuffer.substr(boundPos, endBoundary.length()) == endBoundary) {
                req.SetState(PARSE_COMPLETE);
                rawBuffer.clear(); // We are done!
                return true;
            }

            // 3. Skip the boundary and the \r\n to look at the new file's headers
            size_t headerStart = boundPos + boundary.length() + 2;
            size_t headerEnd = rawBuffer.find("\r\n\r\n", headerStart);

            if (headerEnd == std::string::npos) {
                // We have the boundary, but the headers for this file haven't fully arrived yet.
                // Wait for the next epoll cycle.
                return false; 
            }

            // 4. Extract filename from Content-Disposition
            std::string headers = rawBuffer.substr(headerStart, headerEnd - headerStart);
            std::string extension = ExtractExtensionFromHeaders(headers); // Write a quick helper for this
			
            // 5. Open a NEW temp file for this specific part
            if (req.OpenNewMultipartPart(extension) == false) // e.g., creates ./tmp/part_XXXX
			{
				req.SetErrorCode(HTTP_INTERNAL_SERVER_ERROR);
				req.SetState(PARSE_COMPLETE);
				return (true);
			}

            // Erase everything processed so far (Boundary + Headers + \r\n\r\n)
            rawBuffer.erase(0, headerEnd + 4);
        }
        else 
        {
            // THE SLIDING WINDOW: No boundary found. 
            // We can safely write the buffer to the current file, BUT we must keep the last 
            // N bytes in case a boundary is cut in half across the network!
            size_t safeToWrite = 0;
            if (rawBuffer.length() > boundary.length()) {
                safeToWrite = rawBuffer.length() - boundary.length();
            }

            if (safeToWrite > 0 && req.HasOpenMultipartPart()) {
                req.WriteToCurrentMultipartPart(rawBuffer.substr(0, safeToWrite));
                rawBuffer.erase(0, safeToWrite);
            }

            // Break the while loop and tell epoll we need more data
            return false;
        }
    }
    
    // Check if total received matches content-length
    if (req.GetBodyReceived() >= req.GetContentLength()) {
        req.SetState(PARSE_COMPLETE);
        return true;
    }

    return false;
}

// =========================================================================
// MAIN PARSER LOOP
// =========================================================================
bool RequestParser::Parse(Request& req, std::string& rawBuffer) {
	while (req.GetState() != PARSE_COMPLETE && req.GetState() != PARSE_ERROR) {
		
		// --- PARSE REQUEST LINE ---
		if (req.GetState() == PARSE_REQUEST_LINE) {
			size_t pos = rawBuffer.find("\r\n");
			if (pos == std::string::npos) return false;

			ParseRequestLine(req, rawBuffer.substr(0, pos));
			rawBuffer.erase(0, pos + 2);
			
			if (req.GetState() != PARSE_ERROR) 
				req.SetState(PARSE_HEADERS);
			else
				return true;
		}
		
		// --- PARSE HEADERS ---
		else if (req.GetState() == PARSE_HEADERS) {
			size_t pos = rawBuffer.find("\r\n");
			if (pos == std::string::npos) return false;

			// do we need to delete ? yes so the next time we dont read the same line again
			std::string line = rawBuffer.substr(0, pos);
			rawBuffer.erase(0, pos + 2);

			// need more work on this 
			// do we just get what http 1.0 needed or ust parse every thing ??
			if (line.empty())
			{
				// HTTP/1.0 : reject chunked requests.
				// else we dont now the body length
				if (!req.GetHeader("transfer-encoding").empty() || !req.GetHeader("content-transfer-encoding").empty())
				{
					req.SetErrorCode(HTTP_NOT_IMPLEMENTED);
					req.SetState(PARSE_ERROR);
					return true;
				}

				std::string cl = req.GetHeader("content-length");

				if (!cl.empty())
				{
					for (size_t i = 0; i < cl.size(); ++i)
					{
						if (!std::isdigit(cl[i]))
						{
							req.SetErrorCode(HTTP_BAD_REQUEST);
							req.SetState(PARSE_ERROR);
							return true;
						}
					}

					// Detect Multipart and grab the boundary!
					std::string contentType = req.GetHeader("content-type");
					size_t boundPos = contentType.find("boundary=");
					if (boundPos != std::string::npos) {
						req.SetIsMultipart(true);
						req.SetBoundary("--" + contentType.substr(boundPos + 9));
					} else {
						req.SetIsMultipart(false);
					}

					req.SetContentLength(std::atoi(cl.c_str()));
					// --- FIX: Check for zero-length body immediately ---
					if (req.GetContentLength() == 0)
					{
						req.SetState(PARSE_COMPLETE);
					}
					else
					{
						// after we know that we are going to parse the body we open the file
						if (!req.IsMultipart() && !req.OpenBodyFile()) // well this should be ||   , or we split them each  on with its check cuz normal post body it doesnt detect it 
						{
							req.SetErrorCode(HTTP_INTERNAL_SERVER_ERROR);
							req.SetState(PARSE_ERROR);
							return true;
						}   
						req.SetState(PARSE_BODY);
					}
				}
				else
				{
					// POST requires Content-Length.
					if (req.GetMethod() == HTTP_POST)
					{
						req.SetErrorCode(HTTP_BAD_REQUEST);
						req.SetState(PARSE_ERROR);
						return true;
					}

					req.SetState(PARSE_COMPLETE);
				}
			}
			else
			{
				ParseHeader(req, line);
			}
		}
		
		// --- PARSE BODY ---
		else if (req.GetState() == PARSE_BODY)
		{
			if (req.IsMultipart()) {
                // MULTIPART ON-THE-FLY PARSING
                if (!ParseMultipartBody(req, rawBuffer)) {
                    return false; // Need more data from epoll
                }
            }
			else
			{
				size_t expected = req.GetContentLength();
				size_t remaining = expected - req.GetBodyReceived();

				// Extra Safety Check: If we already have what we need, wrap it up immediately
				if (req.GetBodyReceived() == expected)
				{
					req.CloseBodyFile();
					req.SetState(PARSE_COMPLETE);
					continue; 
				}

				// If rawBuffer is empty, drop out and let epoll wait for more network packets
				if (rawBuffer.empty())
				{
					return false;
				}

				// Determine how much we can write from the current epoll chunk
				size_t toWrite = rawBuffer.length();
				if (toWrite > remaining)
				{
					toWrite = remaining;
				}

				// Write this chunk directly to disk
				if (!req.AppendBody(rawBuffer.data(), toWrite))
				{
					req.SetErrorCode(HTTP_INTERNAL_SERVER_ERROR);
					req.SetState(PARSE_ERROR);
					req.CloseBodyFile(); // Clean up FD immediately on failure
					return true;
				}

				// Erase only what we consumed from the stream buffer
				rawBuffer.erase(0, toWrite);

				// Check if we finally crossed the finish line
				if (req.GetBodyReceived() == expected)
				{
					req.CloseBodyFile(); // Cleanly close the FD here! No lseek needed.
					req.SetState(PARSE_COMPLETE);
				}
				else
				{
					return false; // Body is incomplete, yield back to epoll loop
				}
			}
		}
	}
	return (req.GetState() == PARSE_COMPLETE); // return true parsing is done
}
