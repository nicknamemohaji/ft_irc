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
		IRCClient* AcceptConnection(bool* shouldRead, bool* shouldWrite);
		bool ReadEvent(TCPConnection* conn,
			bool* shouldEndRead, std::set<int> *shouldWriteFDs);
		void WriteEvent(TCPConnection* conn,
			bool* shouldRead, bool* shouldEndWrite);
		void RemoveConnection(TCPConnection* conn, std::set<int> *shouldWriteFDs);

		// public getter
		std::string GetServerName(void) const;
		
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
		// context actions
		// 1. array of method pointers
		void (IRCServer::*Actions[16])(IRCContext& context);
		// 2. action methods
		void ActionAcceptClient(IRCContext& context);
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

		//get pointer from name
		IRCClient* GetClient(const std::string& user_name);
		IRCChannel* GetChannel(const std::string &channel_name);
		
		// channel add and del
		IRCChannel* AddChannel(const std::string &nick_name, const std::string &channel_name, const std::string &channel_password);
		void DelChannel(const std::string &channel_name);
		//check names
		bool IsChannelInList(const std::string& channel_name) const;
		bool IsUserInList(const std::string& user_name) const;
		// commons

		void SendMessageToChannel(enum ChannelSendMode target, IRCContext& context);
		void RmClientFromChanJoined(IRCClient* client);
		void RmClientFromChanInvited(IRCClient* client);

		// ==== disable this constructors ====
		IRCServer(void);
		IRCServer(const IRCServer& other);
		IRCServer& operator=(const IRCServer& other);
};

#endif