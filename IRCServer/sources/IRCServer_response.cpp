#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "IRCServer.hpp"
#include "IRCChannel.hpp"
#include "IRCClient.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

std::string IRCServer::MakeResponse(IRCContext& context)
{
	std::string command;
	if (context.command == UNKNOWN)
		command = context.rawMessage;
	else
		command = IRCContext::ConvertCommandToStr(context.command);
	std::string clientNickname = context.client->GetNickname();
	std::stringstream result;

	// error response
	if (400 <= context.numericResult && context.numericResult < 600)
	{
		result << ":" + _serverName + " "  << std::setw(3) << std::setfill('0') 
			<< context.numericResult << " ";
		switch (context.numericResult)
		{
			// ERR_NOSUCHNICK
			case 401:
				result << clientNickname << " :No such nick/channel";
				break ;
			// ERR_NONICKNAMEGIVEN
			case 431:
				result << clientNickname << " :No nickname given";
				break ;
			// ERR_ERRONEUSNICKNAME
			case 432:
				result << clientNickname << " :Erroneus nickname";
				break ;
			// ERR_NICKNAMEINUSE
			case 433:
				result << clientNickname << " :Nickname is already in use";
				break ;
			//ERR_USERNOTINCHANNEL
			case 441:
				result << clientNickname << " :They aren't on that channel";
				break ;
			// ERR_NOTREGISTERED
			case 451:
				result << clientNickname << " :You have not registered";
				break ;
			// ERR_NEEDMOREPARAMS
			case 461:
				result << clientNickname << " " << command << " :Not enough parameters";
				break ;
			// ERR_ALREADYREGISTERED
			case 462:
				result << clientNickname << " :You may not reregister";
				break ;
			// ERR_PASSWDMISMATCH
			case 464:
				result << clientNickname << " :Password incorrect";
				break ;
			// ERR_CHANOPRIVSNEEDED
			case 482:
				result << clientNickname << " " << context.channel->GetChannelInfo(kChannelName) << " :You're not a channel operator";
				break ;
			// ERR_NOSUCHCHANNEL
			case 403:
				result << clientNickname << " " << context.stringResult << " :No such channel";
				break ;
			// ERR_TOOMANYCHANNELS
			case 405:
				result << clientNickname << " " << context.channel->GetChannelInfo(kChannelName) << " :You have joined too many channels";
				break ;
			//ERR_BADCHANNELKEY* (475) 비밀번호 다름
			case 475:
				result << clientNickname << " " << context.channel->GetChannelInfo(kChannelName) << " :Cannot join channel (+k)";
				break ;
			// *ERR_BANNEDFROMCHAN* (474) 벤된 사용자
			case 474:
				result << clientNickname << " " << context.channel->GetChannelInfo(kChannelName) << " :Cannot join channel (+b)";
				break ;
			// *ERR_CHANNELISFULL* (471) 채널 포화상태
			case 471:
				result << clientNickname << " " << context.channel->GetChannelInfo(kChannelName) << " :Cannot join channel (+l)";
				break ;
			// *ERR_INVITEONLYCHAN* (473) 인바이트 전용채널, 인바이트 안된상태
			case 472:
				result << clientNickname << " " << context.channel->GetChannelInfo(kChannelName) << " :is an unknown char to me";
				break ;
			// *ERR_UNKNOWNMODE* (472) 서버에서 인식할 수 없는 모드문자 사용
			case 473:
				result << clientNickname << " " << context.channel->GetChannelInfo(kChannelName) << " :Cannot join channel (+i)";
				break ;
			// *ERR_BADCHANMASK* (476) 채널이름이 유효하지 않음
			case 476:
				result << context.stringResult << " :Bad Channel Mask";
				break ;
			// ERR_NOTONCHANNEL (442) 채널에 유저가 존재하지 않음
			case 442:
				result << clientNickname << " " << context.channel->GetChannelInfo(kChannelName) << " :You're not on that channel"; 
				// "<client> <channel> :You're not on that channel"
				break;
			// ERR_UNKNOWNCOMMAND
			case 421:
				// FALLTHROUGH
			default:
				result << clientNickname << " " << command << " :Unknown command";
				break ;
		}
	}
	// normal result
	else
	{
		// source ::=  <servername> / ( <nickname> [ "!" <user> ] [ "@" <host> ] )
		// can omit username and hostname for client source
		if (context.source.size() == 0)
			result << ":" << _serverName << " ";
		else
			result << ":" << context.source << " ";
		
		/*
		notes on IRCContext, IRCServer::MakeResponse

		MakeResponse를 호출하는 메소드는 IRCContext의 stringResult 필드에 
		올바른 응답 메시지를 모두 작성해놔야 합니다. (예외상황 제외)
		*/ 
		if (context.numericResult > 0)
			result << std::setw(3) << std::setfill('0') << context.numericResult << " ";
		result << context.stringResult;
	}

	# ifdef DEBUG
	std::cout << "[DEBUG] IRCServer: MakeResponse: from context" << context;
	std::cout << "[DEBUG] IRCServer: MakeResponse: response " << result.str() << std::endl;
	# endif

	result << "\r\n";
	return result.str();
}
