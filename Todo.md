## Webserv
- [] Efficient server/client management
- [] Default + Error pages
- [] Bad response + Status code

## Config File
- [] Need implementaion

## HttpRequest
- [] Need implementaion

## HttpResponse
- [] Need implementaion


- [] You need to manage Correct response, bad response correctly both from normal or CGI request [edge case] `www/` folder

## CGI
### Chunked transfer encoding
- [] For chunked requests (Temp File Method) : 
	- [] HTTP Parser strips out the chunk hex codes, writes only the pure data to the Temp File
	- [] Set the maximum allowed size for client request bodies.
	- [] The CGI should be run in the correct directory for relative path file access.
	- [] 