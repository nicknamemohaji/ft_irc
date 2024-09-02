#ifndef TCPMULTIPLEXER_HPP
#define TCPMULTIPLEXER_HPP

#include <sys/event.h>

#include <map>
#include <vector>
#include <utility>

#include "TCPServer.hpp"
#include "TCPConnection.hpp"

typedef std::vector<struct kevent> KqueueRegisterVec;
typedef std::pair<TCPServer*, TCPConnection*> Client;

/*
* TCP Socket multiplexer
* this class will have kqueue
*/
class TCPMultiplexer
{
	public:
		// ==== methods ====
		// constructor, destructor
		TCPMultiplexer(void);
		TCPMultiplexer(std::vector<TCPServer*>& servers);
		~TCPMultiplexer(void);

		// event handler
		void WaitEvent(void);

		// manage registered servers
		void AddServer(TCPServer* server);
		void RemoveServer(TCPServer* server);


	private:
		// ==== attributes ====
		// clients
		// TODO handle "zombie" connections.. how?
		std::map<int, Client> _clients;
		// servers
		std::map<int, TCPServer*> _servers;
		// kqueue
		int _kqueueFD;
		KqueueRegisterVec _kqueueEvents;


		// ==== methods ====
		// manage registered connections
		void AddConnection(TCPConnection* connection, TCPServer* server);
		void RemoveConnection(Client client, std::set<int>* shouldWriteFDs);

		// kqueue helper
		void AddKevent(int ident, int filter);
		void RemoveKevent(int ident, int filter);

		// ==== constructors ====
		// disable this constructors
		TCPMultiplexer(const TCPMultiplexer& other);
		TCPMultiplexer& operator=(const TCPMultiplexer& other);
};

#endif