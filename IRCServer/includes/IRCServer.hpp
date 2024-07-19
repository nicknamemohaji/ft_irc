#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <string>
#include <utility>
#include <map>

#include "TCPServer.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"

class IRCServer: public TCPServer
{
	public:
		// constructor
		IRCServer(const std::string& port,
			const std::string& servername, const std::string& password);
		~IRCServer(void);

		// event callback
		IRCClient* AcceptConnection(bool& shouldRead, bool& shouldWrite);
		void ReadEvent(TCPConnection* conn,
			bool& shouldEndRead, std::set<int> &shouldWriteFDs);
		void WriteEvent(TCPConnection* conn,
			bool& shouldRead, bool& shouldEndWrite);
		
	protected:

	private:
		// ==== attribute ====
		// server settings
		std::string _serverName;
		std::string _serverPass;
		// channels

		// clients
		std::map<std::string, IRCClient*> _clients;

		// ==== methods ====
		// request
		bool RequestParser(Buffer& buf, IRCContext& context);
		std::string MakeResponse(IRCContext& context);
		/*
		notes on IRCServer::MakeResponse

		when message is sent to a channel, server should broadcast message.
		making message for command `PRIVMSG` should be done in IRCChannel instance.
		*/

		// context actions
		void Context(IRCContext& context);
		// 1. register new client
		void AcceptClient(IRCContext& context);
		// 2. manage existing client
		std::string ManageMOTD(IRCContext& context);
		void ManagePing(IRCContext& context);
		void ManagePong(IRCContext& context);

		// disable this constructors
		IRCServer(void);
		IRCServer(const IRCServer& other);
		IRCServer& operator=(const IRCServer& other);
};

#endif