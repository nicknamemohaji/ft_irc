#include <sys/socket.h>

#include <iostream>
#include <ctime>

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
	std::time_t time = std::time(nullptr);
	char timeString[20];
	std::strftime(
		timeString, 20, "%Y%m%d-%H:%M:%S", std::localtime(&time)
	);
	_startDate = std::string(timeString);

	this->Actions[CAP] = &IRCServer::ActionAcceptClient;
	this->Actions[PASS] = &IRCServer::ActionAcceptClient;
	this->Actions[USER] = &IRCServer::ActionAcceptClient;
	this->Actions[NICK] = &IRCServer::ActionAcceptClient;
	this->Actions[MOTD] = &IRCServer::ActionMOTD;
	this->Actions[PING] = &IRCServer::ActionPING;
	this->Actions[JOIN] = &IRCServer::ActionJOIN;

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
	// new client is registered on _client after registration is complete
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
		// check registration status
		if (conn->GetStatus() != REGISTERED && context.command > NICK)
			throw IRCError::NotRegistered();

		/*
		notes on IRCServer::Actions:

		Actions는 멤버 함수 배열입니다. Actions에 저장되는 순서는 enum IRCCommand를 사용합니다.

		각 메소드는 필요시 IRCContext::client를 참조해 클라이언트에게 메시지를 보낼 수 있는데(IRCClient::Send),
		이 때 IRCContext::FDsPendingWrite에 IRCClient->GetFD의 결과를 추가해야만 정상적으로 메시지가 전송됩니다.

		ex) 
		context.client->Send(MakeResponse(context));
		context.FDsPendingWrite.insert(context.client->GetFD());
		*/
		(this->*(Actions[context.command]))(context);
	}
	catch (const IRCNumeric& e)
	{
		// create error response
		message = conn->ReadRecvBuffer();
		Buffer::iterator it = std::find(message.begin(), message.end(), '\r');
		context.rawMessage = std::string(message.begin(), it);
		context.numericResult = e.code();
		// send error response
		conn->Send(MakeResponse(context));
		shouldWriteFDs.insert(conn->GetFD());
		// clear buffer
		if (it == message.end())
		{
			it = std::find(message.begin(), message.end(), '\n');
			if (it == message.end())
				message.clear();
			else
				message.erase(message.begin(), it + 1);
		}
		else
			message.erase(message.begin(), it + 2);
	}

	message.clear();
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
