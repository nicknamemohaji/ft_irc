#include <unistd.h>
#include <sys/event.h>

#include <map>
#include <set>
#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>

#include "TCPMultiplexer.hpp"
#include "TCPErrors.hpp"
#include "TCPServer.hpp"
#include "TCPConnection.hpp"

// ==== kqueue helper =====
void TCPMultiplexer::AddKevent(int ident, int filter)
{
	struct kevent event;
	EV_SET(
		&event,									// ev
		ident,									// ident: fd
		filter,									// filter: EVFILT_READ = triggers when read is available
												// or EVFILT_WRITE = triggers when write is available
		EV_ADD | EV_ENABLE,						// flag: EV_ADD = register ident, EV_ENABLE = returns on event
		0,										// fflags --- varies on filter (socket type has no fflags)
		0,										// data   --- varies on filter (socket type has no data)
		NULL									// udata: NULL = no timeout (kevent will block)
	);
	_kqueueEvents.push_back(event);
}

void TCPMultiplexer::RemoveKevent(int ident, int filter)
{
	struct kevent event;
	EV_SET(
		&event,									// ev
		ident,									// ident: fd
		filter,									// filter: EVFILT_READ = triggers when read is available
												// or EVFILT_WRITE = triggers when write is available
		EV_DELETE | EV_DISABLE,					// flag: EV_DELETE = dequeue ident, EV_SIABLE = do not return
		0,										// fflags --- varies on filter (socket type has no fflags)
		0,										// data   --- varies on filter (socket type has no data)
		NULL									// udata: NULL = no timeout (kevent will block)
	);
	_kqueueEvents.push_back(event);
}

// ==== constructor, desturctor ====
TCPMultiplexer::TCPMultiplexer(void)
{
	// ***** kqueue *****
	// open kqueue
	_kqueueFD = kqueue();
	if (_kqueueFD < 0)
		throw TCPErrors::SystemCallError("kqueue(2)");

	# ifdef DEBUG
	std::cout << "[DEBUG] TCPMultiplexer: Constructor: kqueue opened in fd " << _kqueueFD << std::endl;
	# endif	

	# ifdef DEBUG
	std::cout << "[INFO] TCPMultiplexer: Constructor: instance created" << std::endl;
	# endif	
}

TCPMultiplexer::TCPMultiplexer(std::vector<TCPServer*>& servers)
{
	// ***** kqueue *****
	// open kqueue
	_kqueueFD = kqueue();
	if (_kqueueFD < 0)
		throw TCPErrors::SystemCallError("kqueue(2)");

	# ifdef DEBUG
	std::cout << "[DEBUG] TCPMultiplexer: Constructor: kqueue opened in fd " << _kqueueFD << std::endl;
	# endif	

	for (std::vector<TCPServer*>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		AddServer(*it);
		# ifdef DEBUG
		std::cout << "[DEBUG] TCPMultiplexer: Constructor: TCPServer (fd " << (*it)->GetFD() << ") dependency injected" << std::endl;
		# endif	
	}

	# ifdef DEBUG
	std::cout << "[INFO] TCPMultiplexer: Constructor: instance created" << std::endl;
	# endif	
}

TCPMultiplexer::~TCPMultiplexer(void)
{
	// ***** kqueue *****
	close(_kqueueFD);
	# ifdef DEBUG
	std::cout << "[DEBUG] TCPMultiplexer: Destructor: closed server socket on fd " << _kqueueFD << std::endl;
	# endif

	# ifdef DEBUG
	std::cout << "[INFO] TCPMultiplexer: Destructor: instance destroyed" << std::endl;
	# endif	
}

// ==== manage server, connection ====
void TCPMultiplexer::AddServer(TCPServer* server)
{
	_servers[server->GetFD()] = server;
	AddKevent(server->GetFD(), EVFILT_READ);
}

void TCPMultiplexer::RemoveServer(TCPServer* server)
{
	std::map<int, TCPServer* >::iterator it = _servers.find(server->GetFD());
	if (it == _servers.end())
	{
		# ifdef DEBUG
		std::cout << "[ERROR] TCPMultiplexer: RemoveServer: server " << server << "is not registered" << std::endl;
		# endif
		return ;
	}
	// remove TCPServer
	_servers.erase(it);
	// remove TCPClient
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); )
	{
		if (it->second.first == server)
			it = _clients.erase(it);
		else
			it++;
	}
}

void TCPMultiplexer::AddConnection(TCPConnection* connection, TCPServer* server)
{
	_clients[connection->GetFD()] = Client(server, connection);
	AddKevent(connection->GetFD(), EVFILT_READ);
}

void TCPMultiplexer::RemoveConnection(TCPConnection* connection)
{
	std::map<int, Client>::iterator it = _clients.find(connection->GetFD());
	if (it == _clients.end())
	{
		# ifdef DEBUG
		std::cout << "[ERROR] TCPMultiplexer: RemoveConnection: connection " << connection << "is not registered" << std::endl;
		# endif
		return ;
	}
	_clients.erase(it);
}

// ==== main event handler function ====
void TCPMultiplexer::WaitEvent(void)
{
	static const int maxEventHandle = 8;
	struct kevent events[maxEventHandle];
	memset(&events[0], 0, sizeof(struct kevent) * maxEventHandle);
	
	if (_servers.size() == 0)
		throw TCPErrors::SocketClosed();

	// enqueue new kqueue events
	int eventCount = kevent(
		_kqueueFD, 								// kqueue fd

		&_kqueueEvents[0],						// event list
		_kqueueEvents.size(),					// size of event list

		&events[0],								// triggered events will be returned here
		maxEventHandle,							// size of event list
		NULL									// timeout = none
	);
	if (eventCount < 0)
		TCPErrors::SystemCallError("kevent(2)");
	// clear kqueue events
	_kqueueEvents.clear();
	
	# ifdef DEBUG
	std::cout << "[DEBUG] TCPMultiplexer: WaitEvent: " << eventCount << " events" << std::endl;
	# endif	

	for (int i = 0; i < eventCount; i++)
	{
		# ifdef DEBUG
		std::cout << "[DEBUG] TCPMultiplexer: WaitEvent: Event #" <<  i << " = FD " << events[i].ident << std::endl;
		std::cout << "[DEBUG] TCPMultiplexer: WaitEvent: type: " << std::hex << events[i].filter << ", flag : " << std::hex << events[i].flags<< std::endl;
		std::cout << std::dec;
		# endif

		if (_clients.find(events[i].ident) == _clients.end()
			&& _servers.find(events[i].ident) == _servers.end())
		{
			// EOF event might occur after FD is closed
			if (!(events[i].flags & EV_EOF))
			{
				# ifdef DEBUG
				std::cout << "[ERROR] TCPMultiplexer: WaitEvent: not registered FD " << events[i].ident << "'s event occured" << std::endl;
				# endif
			}
			continue ;
		}
		if (_servers.find(events[i].ident) != _servers.end())
		{
			// server socket
			TCPServer* server = _servers[events[i].ident];

			// check for error
			if (events[i].flags & EV_EOF)
			{
				RemoveServer(server);
				server->SetFinished();
				
				continue ;
			}
			if (events[i].flags & EV_ERROR)
			{
				# ifdef DEBUG
				std::cout << "[WARNING] TCPMultiplexer: WaitEvent: server error detected" << std::endl;
				# endif

				throw TCPErrors::SocketClosed();
			}

			bool shouldRead;
			bool shouldWrite;
			TCPConnection* conn = server->AcceptConnection(shouldRead, shouldWrite);
			AddConnection(conn, server);
			if (shouldRead)
				AddKevent(conn->GetFD(), EVFILT_READ);
			if (shouldWrite)
				AddKevent(conn->GetFD(), EVFILT_WRITE);

			continue ;
		}

		Client client = _clients[events[i].ident];
		TCPServer* server = client.first;
		TCPConnection* connection = client.second;

		// check for error
		if (events[i].flags & EV_EOF)
		{
			# ifdef DEBUG
			std::cout << "[DEBUG] TCPMultiplexer: WaitEvent: socket closed" << std::endl;
			# endif

			RemoveConnection(connection);
			delete connection;
			continue ;
		}
		if (events[i].flags & EV_ERROR)
		{
			# ifdef DEBUG
			std::cout << "[WARNING] TCPMultiplexer: WaitEvent: client error detected" << std::endl;
			# endif

			RemoveConnection(connection);
			delete connection;
			
			continue ;
		}

		if (events[i].filter == EVFILT_READ)
		{
			// client socket, read
			connection->Recv();
			if (connection->CheckRecvEnd())
			{
				std::set<int> shouldWriteFDs;
				bool shouldEndRead;
				server->ReadEvent(connection, shouldEndRead, shouldWriteFDs);
				if (shouldEndRead)
					RemoveKevent(connection->GetFD(), EVFILT_READ);
				for (std::set<int>::iterator it = shouldWriteFDs.begin(); it != shouldWriteFDs.end(); it++)
					AddKevent(*it, EVFILT_WRITE);
			}
		}
		else
		{
			bool shouldRead;
			bool shouldEndWrite;
			// client socket, write
			server->WriteEvent(connection, shouldRead, shouldEndWrite);
			if (shouldRead)
				AddKevent(connection->GetFD(), EVFILT_READ);
			if (shouldEndWrite)
				RemoveKevent(connection->GetFD(), EVFILT_WRITE);
		}
	}
}