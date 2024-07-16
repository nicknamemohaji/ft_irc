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
		IRCServer(const std::string& port, const std::string& servername);
		~IRCServer(void);

		// event callback
		IRCClient* AcceptConnection(bool& shouldRead, bool& shouldWrite);
		void ReadEvent(TCPConnection* conn,
			bool& shouldEndRead, bool& shouldWrite);
		void WriteEvent(TCPConnection* conn,
			bool& shouldRead, bool& shouldEndWrite);
		
		static std::string MakeNumericResponse(IRCContext& context);
	protected:

	private:
		// ==== attribute ====
		// server name
		std::string _serverName;
		// channels

		// clients

		// ==== methods ====
		// request
		IRCContext RequestParser(const Buffer& buf);

		// context actions
		void Context(IRCContext& context);
		// 1. register new client
		void AcceptClientCAP(IRCContext& context);
		void AcceptClientPASS(IRCContext& context);
		void AcceptClientNICK(IRCContext& context);
		void AcceptClientUSER(IRCContext& context);
		void AcceptClientMOTD(IRCContext& context);
		// 2. manage existing client
		void ManageClientPing(IRCContext& context);
		void ManageClientPong(IRCContext& context);

		// disable this constructors
		IRCServer(void);
		IRCServer(const IRCServer& other);
		IRCServer& operator=(const IRCServer& other);
};

#endif