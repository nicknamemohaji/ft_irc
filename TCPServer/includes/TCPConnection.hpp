#ifndef TCPSERVER_INCLUDES_TCPCONNECTION_HPP_
#define TCPSERVER_INCLUDES_TCPCONNECTION_HPP_

#include <stdint.h>

#include <string>
#include <vector>

#include "TCPServer/includes/TCPErrors.hpp"

typedef std::vector<uint8_t> Buffer;
#define TCPCONN_BUF_SIZE 1024

/*
 * socket wrapper class
 */
class TCPConnection {
 public:
  // constructor, destructor
  explicit TCPConnection(const int sockFd);
  virtual ~TCPConnection(void);
  void Close(void);

  // socket IO for short counts
  // 1. Recv
  void Recv(void);
  Buffer ReadRecvBuffer(void) const;
  void ClearRecvBuffer(void);
  int GetRecvBufferSize(void) const;
  bool CheckRecvEnd(void) const;
  // 2. Send
  template <typename T>
  void Send(const T& message);

  void SendBuffer(void);
  int GetSendBufferSize(void) const;

  // getters
  std::string GetIP(void) const;
  int GetFD(void) const;

  // conversion
  static std::string BufferToString(const Buffer& buf);

 protected:
  // buffer
  // inherited classes should be able to control buffer
  Buffer buf_recv_;
  Buffer buf_send_;

 private:
  // connection socket
  const int socket_;
  std::string client_ip_;
  bool is_open_;

  // disable this constructors
  TCPConnection(void);
  TCPConnection(const TCPConnection& other);
  TCPConnection& operator=(const TCPConnection& other);
};

#include "TCPServer/sources/TCPConnection.tpp"

#endif  // TCPSERVER_INCLUDES_TCPCONNECTION_HPP_
