#ifndef TCPCONNECTION_HPP
#define TCPCONNECTION_HPP

#include <string>
#include <vector>
#include <stdint.h>

typedef std::vector<uint8_t> Buffer;

/*
 * socket wrapper class
 */
class TCPConnection
{
	public:
		// constructor, destructor
		TCPConnection(const int sockFd);
		virtual ~TCPConnection(void);
		void Close(void);

		// socket IO for short counts
		// 1. Recv
		void Recv(void);
		Buffer ReadRecvBuffer(void) const;
		void ClearRecvBuffer(void);
		int GetRecvBufferSize(void) const;
		bool CheckRecvEnd(void) const;
		// 2. Send
		// overload for string or buffer
		void Send(const std::string& message);
		void Send(const Buffer& message);
		// TCPConnection::Send method will only append buffer
		// on EVFILT_WRITE event, multiplexer shoud call
		// TCPConnection::SendBuffer method to actually send buffer
		void SendBuffer(void);
		int GetSendBufferSize(void) const;

		// getters
		std::string GetIP(void) const;
		int GetFD(void) const;

		// conversion
		static std::string BufferToString(const Buffer& buf);
	
	protected:
		
	private:
		// buffer
		Buffer _recvBuf;
		Buffer _sendBuf;

		// connection socket
		const int _clientSocket;
		std::string _clientIP;
		bool _isOpen;

		// disable this constructors
		TCPConnection(void);
		TCPConnection(const TCPConnection& other);
		TCPConnection& operator=(const TCPConnection& other);	
};

#endif
