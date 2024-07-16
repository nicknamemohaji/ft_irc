#include <sys/socket.h>

#include <iostream>
#include <string>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "TCPErrors.hpp"

IRCServer::IRCServer(const std::string& port, const std::string& servername):
	TCPServer(port),
	_serverName(servername)
{
	// TODO validate server name
}

IRCServer::~IRCServer(void)
{
	// TODO close all connections
	// TODO free all memory
}

IRCClient* IRCServer::AcceptConnection(bool& shouldRead, bool& shouldWrite)
{
	if (_finished)
		throw TCPErrors::SocketClosed();
	// make new connection
	int connSock;
	struct sockaddr_storage connAddr;
	socklen_t connLen = sizeof(connAddr);

	if (
		(connSock = accept(
			_serverSock,
			(struct sockaddr *) &connAddr,
			&connLen
		)
		) < 0
	)
		throw TCPErrors::SystemCallError("accept(2)");

	shouldRead = true;
	shouldWrite = false;
	return new IRCClient(connSock);
}

void IRCServer::ReadEvent(TCPConnection* _conn, bool& shouldEndRead, bool& shouldWrite)
{
	IRCClient* conn = static_cast<IRCClient*>(_conn);
	IRCContext context = RequestParser(conn->ReadRecvBuffer());
	Context(context);

	conn->Send(MakeNumericResponse(context));
	shouldEndRead = true;
	shouldWrite = true;
}

void IRCServer::WriteEvent(TCPConnection* _conn, bool& shouldRead, bool& shouldEndWrite)
{
	IRCClient* conn = static_cast<IRCClient*>(_conn);

	conn->SendBuffer();
	if (conn->GetSendBufferSize() == 0)
	{
		shouldRead = true;
		shouldEndWrite = true;
	}
	else
	{
		shouldRead = false;
		shouldEndWrite = false;
	}
}