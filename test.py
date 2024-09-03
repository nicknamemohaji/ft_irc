#! /usr/local/bin/python3

import socket
import re
import sys
import json

class TestLogger:
    # ANSI 색상 코드
    class Colors:
        RESET = "\033[0m"
        CYAN = "\033[96m"
        GREEN = "\033[92m"
        RED = "\033[91m"
        YELLOW = "\033[93m"

    @staticmethod
    def debug(message):
        print(f"{TestLogger.Colors.CYAN}[*] DEBUG{TestLogger.Colors.RESET}: {message}")

    @staticmethod
    def success(message):
        print(f"{TestLogger.Colors.GREEN}[*] SUCCESS{TestLogger.Colors.RESET}: {message}")

    @staticmethod
    def fail(message):
        print(f"{TestLogger.Colors.RED}[*] FAIL{TestLogger.Colors.RESET}: {message}")

class IRCMessage:
    def __init__(self, raw_message):
        self.raw_message = raw_message
        self.source = None
        self.command = None
        self.params = []

        self._parse_message()
    
    def create(source: str | None, command: str | None, params: list[str] | None):
      message = IRCMessage("")
      message.source = source
      message.command = command
      message.params = params
      return message

    def _parse_message(self):
        # 정규표현식으로 source와 command를 파싱
        match = re.match(r'^:(?P<source>([\w]+!~?[\w]+@[\w.]+)|(ft_irc)) +(?P<command>[\d]{3}|[A-Z]+)(?: +(?P<params>.+))?', self.raw_message)
        
        if match:
            self.source = match.group('source')
            self.command = match.group('command')
            params = match.group('params')

            if params:
                self.params = self._parse_params(params)
        if len(self.params) == 0:
          self.params = None

    def _parse_params(self, params):
        # 파라미터 파싱
        if ':' in params:
            before, trailing = params.split(':', 1)
            param_list = before.strip().split()
            param_list.append(trailing)
        else:
            param_list = params.split()
        
        return param_list

    def __eq__(self, other):
        if not isinstance(other, IRCMessage):
            return NotImplemented
        return (
            (self.source == other.source if (self.source is not None and other.source is not None) else True) and
            (self.command == other.command if (self.command is not None and other.command is not None) else True) and
            (self.params == other.params if (self.params is not None and other.params is not None) else True) and
            any([self.source, self.command, self.params]) and
            any([self.source, self.command, self.params])
        )

    def __str__(self):
        # 색상을 사용하여 source, command, params를 구분하여 문자열 반환
        return (
            f"{TestLogger.Colors.CYAN}Source:{TestLogger.Colors.RESET} {self.source} | "
            f"{TestLogger.Colors.GREEN}Command:{TestLogger.Colors.RESET} {self.command}"
            f" | {TestLogger.Colors.YELLOW}Params:{TestLogger.Colors.RESET} "
            + (f"{' '.join(self.params)}" if self.params is not None else f"{self.params}")
        )

class IRCSocket:
    def __init__(self, host: str, port: int):
        self.host = host
        self.port = port
        self.sock = None

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.sock.connect((self.host, self.port))
            TestLogger.debug(f"Connected to {self.host} on port {self.port}")
        except Exception as e:
            TestLogger.fail(f"Error connecting to {self.host} on port {self.port}: {e}")
            sys.exit(1)

    def send(self, message: str):
        try:
            TestLogger.debug(f"Sending: {message.strip()}")
            self.sock.sendall(message.encode('utf-8'))
        except Exception as e:
            TestLogger.fail(f"Error sending data: {e}")
            sys.exit(1)

    def receive(self) -> str:
        try:
            response = self.sock.recv(1024 * 5).decode('utf-8')
            TestLogger.debug(f"Received: {len(response)} bytes")
            return response
        except Exception as e:
            TestLogger.fail(f"Error receiving data: {e}")
            sys.exit(1)

    def close(self):
        if self.sock:
            self.sock.close()
            TestLogger.debug("Socket closed.")

class IRCTest:
    def __init__(self, test_name: str, host: str, port: int, tests: list[tuple[str, None | tuple[IRCMessage]]]):
        self.test_name = test_name
        self.host = host
        self.port = port
        self.tests = tests

    def test(self):
        print('-' * 30, self.test_name, 'test begin', '-' * 30)
        irc_socket = IRCSocket(self.host, self.port)
        irc_socket.connect()

        for test_message, expected_responses in self.tests:
            irc_socket.send(test_message)
            if not expected_responses:
              continue
            responses = irc_socket.receive().split('\r\n')
            for expected_response, response in zip(expected_responses, responses):
              received_message = IRCMessage(response)
              assert received_message == expected_response, f"Test failed: Expected {expected_response}, got {received_message}"

        irc_socket.close()
        del irc_socket
        TestLogger.success(self.test_name)
        print('-' * 30, self.test_name, 'test end', '-' * 30)

def main():
    if len(sys.argv) != 3:
        TestLogger.fail("Usage: python3 test.py <host> <port>")
        sys.exit(1)

    host = sys.argv[1]
    port = int(sys.argv[2])

    tests = None
    with open("test.json", 'r') as f:
      tests = json.load(f)
    
    for test in tests:
      test_cases = list()
      for case in test['tests']:
        message = case['input']
        if case["expected"]:
          expected_responses = [IRCMessage.create(*data) if data else None for data in case["expected"]]
        else:
          expected_responses = None
        test_cases.append((message, expected_responses, ))

      irc_test = IRCTest(test['name'], host, port, test_cases)
      irc_test.test()

if __name__ == "__main__":
    main()
