#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include <netdb.h> 
#include <sys/types.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

#include "TCPServer.hpp"
#include "TCPConnection.hpp"
#include "TCPErrors.hpp"

TCPServer::TCPServer(const std::string& port):
	_finished(false)
{
	int err;
	// ***** socket *****
	// load adress struct
	struct addrinfo addressHints;
	std::memset(&addressHints, 0, sizeof(addressHints));
	addressHints.ai_family = AF_INET;			// only accept IPv4 socket 
												// (im too lazy to accept ipv6)
	addressHints.ai_socktype = SOCK_STREAM;		// accept stream socket
	addressHints.ai_protocol = IPPROTO_TCP;		// accept tcp only
	addressHints.ai_flags = AI_PASSIVE;			// returned socket with AI_PASSICVE flag
												// is suitable for bind(2) call
	struct addrinfo *addressInfo;
	if (
		(err = getaddrinfo(
			NULL,								// node: NULL = host address
			port.c_str(),						// service: port number
			&addressHints,						// hints: get information of this ;)
			&addressInfo						// res
			)
		) != 0
	)
		throw TCPErrors::GAIError(err);
	// request socket
	if (
		(_serverSock = socket(
			addressInfo->ai_family,				// domain: AF
			addressInfo->ai_socktype,			// type: stream/datagram, ...
												// (SOCK_NONBLOCK is not available in mac)
			addressInfo->ai_protocol			// protocol: TCP/UDP, ...
			)
		 ) < 0
	)
		throw TCPErrors::SystemCallError("socket(2)");
	// reuse port
	if (setsockopt(_serverSock, SOL_SOCKET, SO_REUSEADDR, &err, sizeof(err)) != 0)
		throw TCPErrors::SystemCallError("setsockopt(2)");
	// make socket fd nonblocking (SOCK_NONBLOCK is not available in mac)
	if (
	 	(err = fcntl(
 			_serverSock,							// fd
 			F_SETFL,							// command: F_SETFL = set file status
 			O_NONBLOCK							// argument: O_NONBLOCK = non-blocking io on this file
 			)
 		) != 0)
		throw TCPErrors::SystemCallError("fcntl(2)");
				
	// bind port
	if (
		(err = bind(
				_serverSock,						// sockfd
				addressInfo->ai_addr,			// addr
				addressInfo->ai_addrlen			// socklen
			)
		 ) != 0
	)
		throw TCPErrors::SystemCallError("bind(2)");
	// address is not needed anymore!
	freeaddrinfo(addressInfo);
	// set incomming connection
	if (
		(err = listen(
			      _serverSock,					// sockfd
			      SOMAXCONN						// backlog: SOMAXCONN = system max
			)
		) != 0
	)
		throw TCPErrors::SystemCallError("listen(2)");

	# ifdef DEBUG
	std::cout << "[DEBUG] TCPServer: Constructor: socket opened in fd " << _serverSock << std::endl;
	# endif	


}

TCPServer::~TCPServer(void)
{
	// ***** socket *****
	shutdown(_serverSock, SHUT_RDWR);
	close(_serverSock);
	# ifdef DEBUG
	std::cout << "[DEBUG] TCPServer: Destructor: closed server socket on fd " << _serverSock << std::endl;
	# endif	
}

void TCPServer::SetFinished(void)
{
	_finished = true;
}

int TCPServer::GetFD(void) const
{
	return _serverSock;
}