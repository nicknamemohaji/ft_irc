#ifndef IRCSERVER_HPP
#define IRCSERVER_HPP

#include <string>
#include <utility>
#include <map>

#include "TCPServer.hpp"
#include "IRCClient.hpp"
#include "IRCChannel.hpp"
#include "IRCContext.hpp"

typedef std::vector<std::vector<std::string> > StringMatrix;

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
		std::string MakeResponse(IRCContext& context);
		StringMatrix parseStringMatrix(std::deque<std::string> &param);
		std::vector<std::string> PaserComma(std::string& str);
		//get client*
		IRCClient* GetClient(const std::string& user_name);
		
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

		// context actions
		void (IRCServer::*Actions[9])(IRCContext& context);
		// 1. register new client
		void ActionAcceptClient(IRCContext& context);
		// 2. manage existing client
		void ActionMOTD(IRCContext& context);
		void ActionPING(IRCContext& context);
		void ActionJOIN(IRCContext& context);
		void ActionNAMES(IRCContext& context);
		// channel add and del
		IRCChannel* AddChannel(const std::string &nick_name, const std::string &channel_name, const std::string &channel_password);
		void DelChannel(const std::string &channel_name);
		IRCChannel* GetChannel(const std::string &channel_name);	
		//check channel exist
		bool IsChannelInList(const std::string& channel_name) const;
		//check user exist
		bool IsUserInList(const std::string& user_name) const;
		//check channel name
		bool isValidChannelName(const std::string& name) const;
		std::string AddPrefixToChannelName(const std::string& name);
		std::string DelPrefixToChannelName(const std::string& name);
		// disable this constructors
		IRCServer(void);
		IRCServer(const IRCServer& other);
		IRCServer& operator=(const IRCServer& other);
};

#endif