#include <sys/socket.h>

#include "IRCServer.hpp"
#include "IRCClient.hpp"
#include "IRCErrors.hpp"
#include "TCPErrors.hpp"

IRCServer::IRCServer(const std::string& port,
	const std::string& servername, const std::string& password):
	TCPServer(port),
	_serverName(servername),
	_serverPass(password)
{
	// TODO validate server name
}

IRCServer::~IRCServer(void)
{
	// TODO close all connections
	// TODO free all memory
}

/******************/

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

void IRCServer::ReadEvent(TCPConnection* _conn, bool& shouldEndRead, std::set<int> &shouldWriteFDs)
{
	IRCClient* conn = static_cast<IRCClient*>(_conn);
	Buffer message = conn->ReadRecvBuffer();

	# ifdef DEBUG
	std::cout << "[DEBUG] IRCServer: ReadEvent: dump (" << message << ")" << std::endl;
	# endif

	IRCContext context(shouldWriteFDs);
	context.server = this;
	context.client = conn;

	try
	{
		// TODO 417 ERR_INPUTTOLONG
		if (!RequestParser(message, context))
			return ;
		Context(context);
	}
	catch (const IRCNumeric& e)
	{
		message = conn->ReadRecvBuffer();
		Buffer::iterator it = std::find(message.begin(), message.end(), '\r');
		if (context.command == UNKNOWN)
			context.rawMessage = std::string(message.begin(), it);
		
		context.numericResult = e.code();
		conn->Send(MakeResponse(context));
		shouldWriteFDs.insert(conn->GetFD());

		if (it == message.end())
		{
			it = std::find(message.begin(), message.end(), '\n');
			if (it == message.end())
				message.erase(message.begin(), it + 1);
			else
				message.clear();
		}
		else
			message.erase(message.begin(), it + 2);
	}

	conn->OverwriteRecvBuffer(message);
	shouldEndRead = false;
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
