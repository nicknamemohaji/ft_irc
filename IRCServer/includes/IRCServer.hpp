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
typedef std::map<std::string, IRCClient*> IRCServerClients;

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
		void RemoveConnection(TCPConnection* conn, std::set<int> &shouldWriteFDs);
		
		// expose these methods:: 
		std::vector<std::string> ParserSep(const std::string& str, const std::string& sep);
		//get client*
		IRCClient* GetClient(const std::string& user_name);
		//RPL
		void RPL_NOTOPIC(IRCContext& context);
		void RPL_TOPIC(IRCContext& context);
		void RPL_TOPICWHOTIME(IRCContext& context);
		void RPL_ENDOFNAMES(IRCContext& context);
		void RPL_NAMREPLY(IRCContext& context);
		void RPL_CREATIONTIME(IRCContext& context);
		void RPL_CHANNELMODEIS(IRCContext& context);
		void RPL_INVITING(IRCContext& context);
		void RPL_INVITED(IRCContext& context);
		void ErrorSender(IRCContext context, unsigned int errornum);
		
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
		void AddNewLineToBuffer(Buffer& message);
		// context actions
		void (IRCServer::*Actions[15])(IRCContext& context);
		// 1. register new client
		void ActionAcceptClient(IRCContext& context);
		// 2. manage existing client
		void ActionMOTD(IRCContext& context);
		void ActionPING(IRCContext& context);
		void ActionQUIT(IRCContext& context);
		void ActionJOIN(IRCContext& context);
		void ActionNAMES(IRCContext& context);
		void ActionMODE(IRCContext& context);
		void ActionPART(IRCContext& context);
		void ActionTOPIC(IRCContext& context);
		void ActionKICK(IRCContext& context);
		void ActionPRIVMSG(IRCContext& context);
		void ActionINVITE(IRCContext& context);
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
		// commons
		void SendMessageToChannel(IRCContext& context, bool sendAlso);

		// disable this constructors
		IRCServer(void);
		IRCServer(const IRCServer& other);
		IRCServer& operator=(const IRCServer& other);
};

#endif