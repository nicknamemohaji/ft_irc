#include "IRCServer.hpp"
#include "IRCClient.hpp"

#include <sys/socket.h>

#include <iostream>
#include <ctime>
#include <algorithm>

#include "IRCTypes.hpp"
#include "IRCRequestParser.hpp"
#include "IRCResponseCreator.hpp"
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
	this->Actions[QUIT] = &IRCServer::ActionQUIT;
	this->Actions[JOIN] = &IRCServer::ActionJOIN;
	this->Actions[NAMES] = &IRCServer::ActionNAMES;
	this->Actions[MODE] = &IRCServer::ActionMODE;
	this->Actions[PART] = &IRCServer::ActionPART;
	this->Actions[TOPIC] = &IRCServer::ActionTOPIC;
	this->Actions[KICK] = &IRCServer::ActionKICK;
	this->Actions[PRIVMSG] = &IRCServer::ActionPRIVMSG;
	this->Actions[INVITE] = &IRCServer::ActionINVITE;


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

  // handle short count
  // TODO(kyungjle) raise ERR_LINETOOLONG(417) for big chunks
  if (std::find(message.begin(), message.end(), '\r') == message.end()
    && std::find(message.begin(), message.end(), '\n') == message.end())
    return;

  // match CRLF set and ignore empty message
  IRC_request_parser::AddNewLineToBuffer(&message);
	if (*(message.begin()) == '\r')
	{
		message.erase(message.begin(), message.begin() + 2);
		conn->OverwriteRecvBuffer(message);
		return ;
	}

  IRCContext context(shouldWriteFDs);
  context.server = this;
  context.client = conn;


  try
  {
    IRCCommand _command;
    IRCParams _params;
    std::cout << "1" << std::endl;
    if (!IRC_request_parser::ParseMessage(&message, &_command, &_params))
      throw IRCError::UnknownCommand();  // TODO(kyungjle) dont use exception
    context.command = _command;
    context.params = _params;

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
		IRC_request_parser::AddNewLineToBuffer(&message);
		Buffer::iterator it = std::find(message.begin(), message.end(), '\r');
		context.stringResult = std::string(message.begin(), it);
		// send error response
		context.numericResult = e.code();
    context.createSource = false;
		conn->Send(IRC_response_creator::MakeResponse(context));
		shouldWriteFDs.insert(conn->GetFD());

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
		if (conn->GetStatus() == PENDING_QUIT)
		{
			_clients.erase(_clients.find(conn->GetNickname()));
			conn->Close();
			shouldRead = false;
			shouldEndWrite = true;
			return ;
		}
		shouldRead = true;
		shouldEndWrite = true;
	}
	else
	{
		shouldRead = false;
		shouldEndWrite = false;
	}
}

void IRCServer::RemoveConnection(TCPConnection* _conn, std::set<int> &shouldWriteFDs)
{
	IRCClient* conn = static_cast<IRCClient*>(_conn);

	// if user is not registered, delete client instance immediately
	if (GetClient(conn->GetNickname()) == NULL)
	{
		conn->Close();
		return ;
	}

	IRCContext context(shouldWriteFDs);
	context.client = conn;
	context.params.push_back("Client quited unexpectidly");
	ActionQUIT(context);

	_clients.erase(_clients.find(conn->GetNickname()));
	conn->Close();

	return ;
}

/***********************/

IRCChannel* IRCServer::AddChannel(const std::string &nick_name, const std::string &channel_name, const std::string &channel_password){
	#ifdef COMMAND
	std::cout << "create channel " << channel_name << " password is " << channel_password << std::endl;
	#endif
	IRCChannel *ret;
	if(channel_password == "" || channel_password == "x")
		ret = new IRCChannel(nick_name,channel_name);
	else
		ret = new IRCChannel(nick_name,channel_name,channel_password);
	_channels[channel_name] =  ret;
	return ret;
	
}

void IRCServer::DelChannel(const std::string &channel_name){
	std::map<std::string, IRCChannel*>::iterator it = _channels.find(channel_name);
	if(it == _channels.end())
		return;
	delete it->second;
	_channels.erase(it);
}

IRCChannel* IRCServer::GetChannel(const std::string& channel_name) {
	std::map<std::string, IRCChannel*>::const_iterator it = _channels.find(channel_name);
	if(it != _channels.end())
		return it->second;
	return NULL;
}

bool IRCServer::IsChannelInList(const std::string& channel_name) const{
	return _channels.find(channel_name) != _channels.end();
}

bool IRCServer::IsUserInList(const std::string& user_name) const{
	return _clients.find(user_name) != _clients.end();
}

IRCClient* IRCServer::GetClient(const std::string& user_name){
	# ifdef COMMAND
		std::cout << "client size in server  " << _clients.size() <<std::endl;
	# endif
	std::map<std::string, IRCClient*>::const_iterator it = _clients.find(user_name);
	if(it == _clients.end())
		return NULL;
	return it->second;
}

std::string IRCServer::GetServerName(void) const
{
	return _serverName;
}
