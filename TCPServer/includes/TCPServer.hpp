#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <string>
#include <set>

#include "TCPConnection.hpp"

/*
* Interface class for TCP server
* well this is not pure interface ;(
*/
class TCPServer
{
	public:
		// constructor, destuctor
		virtual ~TCPServer(void);
		void SetFinished(void);

		// actions
		virtual TCPConnection* AcceptConnection
			(bool* shouldRead, bool* shouldWrite) = 0;
		virtual bool ReadEvent(TCPConnection* conn,
			bool* shouldEndRead, std::set<int>* shouldWriteFDs) = 0;
		virtual void WriteEvent(TCPConnection* conn,
			bool* shouldRead, bool* shouldEndWrite) = 0;
		virtual void RemoveConnection(
			TCPConnection* conn, std::set<int>* shouldWriteFDs) = 0;

		// getters
		int GetFD(void) const;

	protected:
		// socket
		int _serverSock;

		// finished state
		bool _finished;

		// constructor
		TCPServer(const std::string& port);
	private:
};

#endif