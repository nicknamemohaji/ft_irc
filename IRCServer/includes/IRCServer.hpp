#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <string>
#include <utility>
#include <map>

#include "TCPServer.hpp"
#include "IRCClient.hpp"
#include "IRCChannel.hpp"
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
		std::string _startDate;
		// channels
		std::map<std::string, IRCChannel*> _channels;

		// clients
		std::map<std::string, IRCClient*> _clients;

		// ==== methods ====
		// request, response
		bool RequestParser(Buffer& buf, IRCContext& context);
		std::string MakeResponse(IRCContext& context);

		// context actions
		void (IRCServer::*Actions[6])(IRCContext& context);
		// 1. register new client
		void ActionAcceptClient(IRCContext& context);
		// 2. manage existing client
		void ActionMOTD(IRCContext& context);
		void ActionPING(IRCContext& context);

		// disable this constructors
		IRCServer(void);
		IRCServer(const IRCServer& other);
		IRCServer& operator=(const IRCServer& other);
};

#endif