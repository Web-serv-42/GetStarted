#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

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
	int					sockfd;

	memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6 => AF_INET AF_INET6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets. Or SOCK_DGRAM for UDP
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
	
	if  ((status = getaddrinfo(NULL, "1337", &hints, &servinfo)) != 0)
	{
		std::cerr << gai_strerror(status) << std::endl;
		exit(1);
	}

	// Create the endpoint wich we can use later 
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	// Bind the endpoint with a port specified by the OS
	status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	if (status != 0)
	{
		std::cerr << gai_strerror(status) << std::endl;
		exit(1);
	}

	// Clean Recources
	freeaddrinfo(servinfo);
	return 0;
}
