#ifndef TCPSERVER_INCLUDES_TCPERRORS_HPP_
#define TCPSERVER_INCLUDES_TCPERRORS_HPP_

#include <stdexcept>
#include <string>

namespace TCPErrors {
class SystemCallError;
class GAIError;
class SocketClosed;
}  // namespace TCPErrors

class TCPErrors::SystemCallError : public std::runtime_error {
 public:
  explicit SystemCallError(const std::string &call);
};

class TCPErrors::GAIError : public std::runtime_error {
 public:
  explicit GAIError(const int err);
};

class TCPErrors::SocketClosed : public std::runtime_error {
 public:
  SocketClosed(void);
};

#endif  // TCPSERVER_INCLUDES_TCPERRORS_HPP_
