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
	AddNewLineToBuffer(message);

	# ifdef DEBUG
	std::cout << "[DEBUG] IRCServer: ReadEvent: dump (" << message << ")" << std::endl;
	# endif

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
		AddNewLineToBuffer(message);
		Buffer::iterator it = std::find(message.begin(), message.end(), '\r');
		context.rawMessage = std::string(message.begin(), it);
		// send error response
		context.numericResult = e.code();
		conn->Send(MakeResponse(context));
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

	// check if user is not deleted
	if (GetClient(conn->GetNickname()) == NULL)
		return ;

	IRCContext context(shouldWriteFDs);

	// TODO remove repeated code segment (QUIT.cpp > ActionQUIT)
	IRCClientChannels channels = conn->ListChannels();
	for (IRCClientChannels::iterator it = channels.begin(); it != channels.end(); it++)
	{
		// broadcast
		context.numericResult = -1;
		context.client = conn;
		context.channel = it->second;
		context.stringResult = "Error: Client quited unexpectidly";
		context.createSource = true;
		SendMessageToChannel(context, false);
		// change name from channel
		context.channel->DelChannelUser(conn->GetNickname());
	}

	// TODO remove repeated code segment (IRCServer_server.cpp > WriteEvent)
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
	if(channel_password == "")
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

bool IRCServer::isValidChannelName(const std::string &name) const {
	if(name.size() > 10 || name.size() < 2)
		return false;
	if(name[0] != '#')
		return false;
	if(std::string::npos != name.find('#',1))
		return false;
	for(unsigned int i = 1; i < name.size(); ++i)
	{
		if(!std::isalnum(static_cast<unsigned char>(name[i])))
			return false;
	}
	return true;
}

std::vector<std::string> IRCServer::ParserSep(const std::string& str, const std::string& sep)
{
    std::vector<std::string> param;
    size_t start = 0;
    size_t end = str.find(sep);
    
    while (end != std::string::npos)
    {
        param.push_back(str.substr(start, end - start));
        start = end + sep.length();
        end = str.find(sep, start);
    }
    
    // 마지막 요소 추가
    param.push_back(str.substr(start));
    
    return param;
}

std::string IRCServer::AddPrefixToChannelName(const std::string& name){
	if(name.size() < 1 || name[0] == '#')
		return name;
	return "#" + name;
}

std::string IRCServer::DelPrefixToChannelName(const std::string& name){
	if(name.size() < 1 || name[0] != '#')
		return name;
	return name.substr(1);
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