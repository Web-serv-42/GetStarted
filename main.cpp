#include <iostream>
#include <string>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	struct addrinfo		hints;
	struct addrinfo*	servinfo;  // will point to the results
	int					status;

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
	
	if  ((status = getaddrinfo(NULL, "1337", &hints, &servinfo)) != 0)
	{
		std::cerr << gai_strerror(status) << std::endl;
	}
	freeaddrinfo(servinfo);
	return 0;
}
