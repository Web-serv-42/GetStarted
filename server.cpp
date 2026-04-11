#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

#define MYPORT "3490"
#define BACKLOG 20

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	struct addrinfo		hints;
	struct addrinfo*	servinfo;  // will point to the results
	int					status;
	int					sockfd, client_fd;
	struct sockaddr_storage	their_addr;
	socklen_t				addr_size;

	memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6 => AF_INET AF_INET6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets. Or SOCK_DGRAM for UDP
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
	
	if  ((status = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0)
	{
		std::cerr << gai_strerror(status) << std::endl;
		exit(1);
	}

	// Create the endpoint wich we can use later 
	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockfd == -1) {
        std::cerr << "socket error: " << strerror(errno) << std::endl;
        exit(1);
    }

	// Bind the endpoint with a port specified by the OS
	status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	if (status != 0)
	{
		std::cerr << strerror(errno) << std::endl;
		exit(1);
	}

	// Clean Recources
	freeaddrinfo(servinfo);

	// Listening for new a connection
	status = listen(sockfd, BACKLOG);
	if (status != 0)
	{
		std::cerr << strerror(errno) << std::endl;
		exit(1);
	}

	std::cout << "Server is waiting for connections on port " << MYPORT << "..." << std::endl;

	addr_size = sizeof(their_addr);
	char	buf[1024];

	// Listening loop
	while (true)
	{
		// Accept an incoming call
		client_fd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
		if (client_fd == -1) {
			std::cerr << "accept error: " << strerror(errno) << std::endl;
			exit(1);
		}

		// Ready to communicate on socket descriptor client_fd
		memset(buf, 0, sizeof(buf)); // Clear buffer

		ssize_t bytes_received = recv(client_fd, buf, sizeof(buf) - 1, 0);
		
		if (bytes_received == -1)
		{
			std::cerr << "recv error: " << strerror(errno) << std::endl;
		}
		else if (bytes_received == 0)
		{
			std::cout << "Client closed the connection." << std::endl;
		}
		else
		{
			std::cout << "Server received: " << buf << std::endl;
		}
	}

	close(client_fd);
	close(sockfd);

	return 0;
}
