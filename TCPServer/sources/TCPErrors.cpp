#include <stdexcept>
#include <string>

#include <cerrno>
#include <netdb.h> 

#include "TCPErrors.hpp"

TCPErrors::SystemCallError::SystemCallError(const std::string call):
	std::runtime_error(
		call + "error = " + std::strerror(errno)
	)
{
}

TCPErrors::GAIError::GAIError(const int err):
	std::runtime_error(
		std::string("getaddrinfo(3) error = ") + gai_strerror(err)
	)
{
}

TCPErrors::SocketClosed::SocketClosed(void):
	std::runtime_error("socket is closed")
{	
}