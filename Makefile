NAME := ircserv

SRCS_TCPSERVER := 	TCPServer/sources/TCPMultiplexer.cpp \
					TCPServer/sources/TCPConnection.cpp \
					TCPServer/sources/TCPErrors.cpp \
					TCPServer/sources/TCPServer.cpp

# TODO: add sources directrly
SRCS_IRCSERVER :=	$(wildcard IRCServer/sources/*.cpp)\
					$(wildcard IRCServer/sources/Command/*.cpp)\
					$(wildcard IRCServer/sources/RPL/*.cpp)

SRCS := $(SRCS_TCPSERVER) $(SRCS_IRCSERVER) server.cpp

OBJS := $(SRCS:.cpp=.o)

INCLUDES :=	-I TCPServer/includes \
			-I IRCServer/includes

C++ := c++
CXXFLAGS := -Wall -Werror -Wextra -std=c++98 $(INCLUDES) #-g -fsanitize=address -D COMMAND -D DEBUG

all: $(NAME)

$(NAME): $(OBJS)
	@echo linking objects
	@$(C++) $(CXXFLAGS) -o $@ $(OBJS)
	@echo done!

%.o: %.cpp
	@echo compiling $<
	@$(CXX) $(CXXFLAGS) -c $< -o $*.o

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(NAME)

re: fclean all


.PHONY: all clean fclean re