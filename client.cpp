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

	if  ((status = getaddrinfo("127.0.0.1", MYPORT, &hints, &servinfo)) != 0)
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

	// Connect to the server
	status = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	if 
	(status != 0)
	{
		std::cerr << strerror(errno) << std::endl;
		close(sockfd);
		exit(1);
	}

	// Clean Recources
	freeaddrinfo(servinfo);

	// Ready to communicate on socket descriptor new_fd
	const char*	msg = "Khello Guys!";
	ssize_t bytes_sent =  send(sockfd, (const void*)msg, strlen(msg), 0);

	if (bytes_sent == -1) {
        std::cerr << "send error: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Client sent " << bytes_sent << " bytes successfully." << std::endl;
    }

	close(sockfd);
	return 0;
}
