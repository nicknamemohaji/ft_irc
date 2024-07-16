#ifndef TCPERRORS_HPP
#define TCPERRORS_HPP

#include <stdexcept>
#include <string>

namespace TCPErrors
{
	class SystemCallError;
	class GAIError;
	class SocketClosed;
}

class TCPErrors::SystemCallError: public std::runtime_error
{
	public:
		SystemCallError(const std::string call);
};

class TCPErrors::GAIError: public std::runtime_error
{
	public:
		GAIError(const int err);
};

class TCPErrors::SocketClosed: public std::runtime_error
{
	public:
		SocketClosed(void);
};

#endif
