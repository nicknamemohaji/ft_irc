#ifndef IRCRRORS_HPP
#define IRCRRORS_HPP

#include <stdexcept>
#include <string>

namespace IRCError
{
	class NoSuchNick;			// 401
	class NoSuchChannel;		// 403
	class CanNotSendToChan;		// 404
	class TooManyChannel;		// 405
	class UnknownCommand;		// 421
	class UserNotInChannel;		// 441
	class NotOnChannel;			// 442
	class ChannelIsFull;		// 471
	class UnknownModeChar;		// 472
	class InviteOnly;			// 473
	class BadChannelKey;		// 475
	class BadChannelName;		// 476
	class ChangeNoPrivesneed;	// 482

};

// base error
class IRCNumeric: public std::logic_error
{
	public:
		IRCNumeric(const int code):
			std::logic_error("IRC ERROR"),
			_returnCode(code)
		{
		}

		int code(void) const
		{
			return _returnCode;
		}


	private:
		int _returnCode;
};

/***************/
// ERR_NOSUCHNICK
class IRCError::NoSuchNick: public IRCNumeric
{
	public:
		NoSuchNick(void):
			IRCNumeric(401)
		{
		}
};

// ERR_NOSUCHCHANNEL
class IRCError::NoSuchChannel: public IRCNumeric
{
	public:
		NoSuchChannel(void):
			IRCNumeric(403)
		{
		}
};

// ERR_CANNOTSENDTOCHAN (404): 메시지를 채널에 전달할 수 없음
class IRCError::CanNotSendToChan: public IRCNumeric
{
	public:
		CanNotSendToChan(void):
			IRCNumeric(404)
		{
		}
};

// ERR_TOOMANYCHANNELS
class IRCError::TooManyChannel: public IRCNumeric
{
	public:
		TooManyChannel(void):
			IRCNumeric(405)
		{
		}
};
// ERR_UNKNOWNCOMMAND
class IRCError::UnknownCommand: public IRCNumeric
{
	public:
		UnknownCommand(void):
			IRCNumeric(421)
		{
		}
};

// ERR_USERNOTINCHANNEL
class IRCError::UserNotInChannel: public IRCNumeric
{
	public:
		UserNotInChannel(void):
			IRCNumeric(441)
		{
		}
};

// ERR_NOTONCHANNEL
class IRCError::NotOnChannel: public IRCNumeric
{
	public:
		NotOnChannel(void):
			IRCNumeric(442)
		{
		}
};

// *ERR_CHANNELISFULL* (471) 채널 포화상태
class IRCError::ChannelIsFull: public IRCNumeric
{
	public:
		ChannelIsFull(void):
			IRCNumeric(471)
		{
		}
};
// *ERR_UNKNOWNMODE* (472) 서버에서 인식할 수 없는 모드문자 사용
class IRCError::UnknownModeChar: public IRCNumeric
{
	public:
		UnknownModeChar(void):
			IRCNumeric(472)
		{
		}
};
// *ERR_INVITEONLYCHAN* (473) 인바이트 전용채널, 인바이트 안된상태
class IRCError::InviteOnly: public IRCNumeric
{
	public:
		InviteOnly(void):
			IRCNumeric(473)
		{
		}
};

//ERR_BADCHANNELKEY* (475) 비밀번호 다름
class IRCError::BadChannelKey: public IRCNumeric
{
	public:
		BadChannelKey(void):
			IRCNumeric(475)
		{
		}
};

// *ERR_BADCHANMASK* (476) 채널이름이 유효하지 않음
class IRCError::BadChannelName: public IRCNumeric
{
	public:
		BadChannelName(void):
			IRCNumeric(476)
		{
		}
};


// ERR_CHANOPRIVSNEEDED
class IRCError::ChangeNoPrivesneed: public IRCNumeric
{
	public:
		ChangeNoPrivesneed(void):
			IRCNumeric(482)
		{
		}
};
#endif