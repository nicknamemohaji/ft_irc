#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <arpa/inet.h>

#include "TCPConnection.hpp"
#include "TCPErrors.hpp"

// ===== constructor, destructor =====
TCPConnection::TCPConnection(const int sockFd):
	_clientSocket(sockFd),
	_isOpen(true)
{
	/***** fetch client information *****/
	// use getpeername(2) to fetch peer's information
	struct sockaddr_storage clientInfo;
	socklen_t clientInfoLen = sizeof(clientInfo); 
	int err = getpeername(
		_clientSocket,					// sockfd
		(struct sockaddr *)&clientInfo,	// addr
		&clientInfoLen					// len
	);
	if (err != 0)
		throw TCPErrors::SystemCallError("getpeername(2)");
	// use inet_ntop(3) to convert peer's ip address to c-style string
	char buf[256];
	std::memset(buf, 0, sizeof(buf));
	if (
		inet_ntop(
			AF_INET, 
			&(((struct sockaddr_in*)&clientInfo)->sin_addr), 
			buf,
			sizeof(buf)
		) == NULL
	)
		throw TCPErrors::SystemCallError("inet_ntop(3)");
	// save clientIp
	_clientIP = buf;

	/***** set socket fd *****/
	// make fd O_NONBLOCK
	if (
	 	(err = fcntl(
 			_clientSocket,				// fd
 			F_SETFL,					// command: F_SETFL = set file status
 			O_NONBLOCK					// argument: O_NONBLOCK = non-blocking io on this file
 			)
 		) != 0)
		throw TCPErrors::SystemCallError("fcntl(2)");

	# ifdef DEBUG
	std::cout << "[INFO] TCPConnection: Constructor: instnace created for fd " << _clientSocket << ", ip " << _clientIP << std::endl;
	# endif
}

TCPConnection::~TCPConnection(void)
{
	/***** close TCPconnection *****/
	Close();

	# ifdef DEBUG
	std::cout << "[INFO] TCPConnection: destructor: instnace destroyed for fd " << _clientSocket << ", ip " << _clientIP << std::endl;
	# endif
}

// semi-destructor :)
void TCPConnection::Close(void)
{
	if (!_isOpen)
		return ;
	
	// send FIN packet
	if (shutdown(
		_clientSocket,					// sockfd
		SHUT_RDWR						// how
	) != 0)
		TCPErrors::SystemCallError("shutdown(2)");
	// close socket
	if (close(_clientSocket))
		TCPErrors::SystemCallError("close(2)");

	// mark status
	_isOpen = false;
}


// ===== socket IO actions =====
// 1. Recv related methods
void TCPConnection::Recv(void)
{
	if (!_isOpen)
		throw TCPErrors::SocketClosed();
	

	uint8_t buf[512];
	std::memset(buf, 0, sizeof(buf));

	int err = recv(
		_clientSocket,					// sockfd
		buf,							// buf
		sizeof(buf),					// len
		0								// flags: none
	);

	if (err == -1)
		throw TCPErrors::SystemCallError("recv(2)");
	else if (err == 0)
	{
		Close();
		return ;
	}

	_recvBuf.insert(_recvBuf.end(), buf, buf + err);

	# ifdef DEBUG
	std::cout << "[DEBUG] TCPConnection: Recv: received " << err << "bytes from client " << _clientIP << std::endl;
	# endif
}

int TCPConnection::GetRecvBufferSize(void) const
{
	return _recvBuf.size();
}

Buffer TCPConnection::ReadRecvBuffer(void) const
{
	return _recvBuf;
}

void TCPConnection::ClearRecvBuffer(void)
{
	_recvBuf.clear();
}

bool TCPConnection::CheckRecvEnd(void) const
{
	if (!_isOpen)
		throw TCPErrors::SocketClosed();

	uint8_t byte;
	int bufferSize = recv(
		_clientSocket,					// sockfd
		&byte,							// buf
		sizeof(byte),					// len
		MSG_PEEK						// flags: MSG_PEEK = just check buffer
	);

	return bufferSize != sizeof(byte);
};

// 2. Send related methods
void TCPConnection::Send(const std::string& message)
{
	if (!_isOpen)
		throw TCPErrors::SocketClosed();
	
	_sendBuf.insert(_sendBuf.end(), message.begin(), message.end());
	// SendBuffer();
}

void TCPConnection::Send(const Buffer& message)
{
	if (!_isOpen)
		throw TCPErrors::SocketClosed();
	
	_sendBuf.insert(_sendBuf.end(), message.begin(), message.end());
}

void TCPConnection::SendBuffer(void)
{
	if (!_isOpen)
		throw TCPErrors::SocketClosed();
	
	int err = send(
		_clientSocket,		// sockfd
		_sendBuf.data(),	// msg
		_sendBuf.size(),	// len
		0					// flags: 0
							// (MSG_NOSIGNAL is not available in mac)
	);
	// error occured
	if (err == -1)
		throw TCPErrors::SystemCallError("send(2)");
	// sent succesfully
	else if (err == (int) _sendBuf.size())
		_sendBuf.clear();
	// short count
	else
		_sendBuf.erase(_sendBuf.begin(), _sendBuf.end());

	# ifdef DEBUG
	std::cout << "[DEBUG] TCPConnection: SendBuffer: sent " << err << " bytes to client " << _clientIP << std::endl;
	std::cout << "[DEBUG] TCPConnection: SendBuffer: " << _sendBuf.size() << " bytes left" << _clientIP << std::endl;
	# endif
}

int TCPConnection::GetSendBufferSize(void) const
{
	return _sendBuf.size();
}

// ===== getters =====
std::string TCPConnection::GetIP(void) const
{
	if (!_isOpen)
		throw TCPErrors::SocketClosed();

	return _clientIP;
}

int TCPConnection::GetFD(void) const
{
	return _clientSocket;
}

// ===== utility function =====

std::string TCPConnection::BufferToString(const Buffer& buf)
{
	std::string ret;
	ret.assign(buf.begin(), buf.end());
	return ret;
}

std::ostream& operator<< (std::ostream& ostream, const Buffer& buffer)
{
	# ifdef DEBUG
	ostream << std::endl;
	for (Buffer::const_iterator it = buffer.begin(); it != buffer.end(); it++)
		ostream << "(" << (unsigned int) *it <<  "),";
	# else
	std::string ret;
	ret.assign(buffer.begin(), buffer.end());
	ostream << ret;
	# endif
	ostream << std::endl;
	return ostream;
}