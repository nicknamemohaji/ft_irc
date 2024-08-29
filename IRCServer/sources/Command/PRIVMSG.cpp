#include "IRCServer.hpp"
#include "IRCClient.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <deque>
#include <algorithm>

#include "IRCResponseCreator.hpp"
#include "IRCChannel.hpp"
#include "IRCContext.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionPRIVMSG(IRCContext& context){
		//  param size =1 channel left;
		# ifdef PCOMMAND
			std::cout << " privmsg param size = " << context.params.size() <<std::endl;
			for(unsigned int i = 0; i < context.params.size(); i++)
				std::cout << "param " << i << " " << context.params[i] << std::endl;
			std::cout << " end. param privmsg "<< std::endl;
			if(context.params.size() >0)
				std::cout << context.params[0].substr(1) << std::endl;
		# endif
		std::string target;
		std::string msg;
		IRCChannel *channel;
		if(context.params.size() != 2)
			throw IRCError::MissingParams();
		target = context.params[0];
		msg = context.params[1];

		context.numericResult = -1;
		context.createSource = true;
		context.stringResult  = "PRIVMSG " + target +" :" + msg;

		if (IsChannelInList(target) || 
			(target.size() > 1 && target[0] == '@' && IsChannelInList(target.substr(1)))
		)
		{
			# ifdef PCOMMAND
				std::cout << "send to channel" << std::endl;
			# endif

			enum ChannelSendMode send_mode = kChanSendModeToExceptMe;
			channel = GetChannel(target);
			if (channel == NULL)
			{
				target = target.substr(1);
				channel = GetChannel(target);
				send_mode = kChanSendModeToOper;
			}
			context.channel = channel;
			
			if((channel->CheckChannelMode(kPassword) || channel->CheckChannelMode(kInvite)) && !channel->IsInChannel(context.client->GetNickname()))
			{
				context.stringResult = target;
				throw IRCError::CanNotSendToChan();
			}

			SendMessageToChannel(send_mode, context);
		}
		else if(IsUserInList(target)){
			IRCClient *user_target = GetClient(target);
			if(!user_target)
				throw IRCError::NoSuchNick();
			user_target->Send(IRCResponseCreator::MakeResponse(context));
			context.FDsPendingWrite.insert(user_target->GetFD());
		}
		else{
			context.stringResult = target;
			throw IRCError::CanNotSendToChan();
		}
}

/* 
/msg #aa,#bb asd -> #bb asd
/msg somin,chansu,miki aa -> miki aa
/msg @#aa asd -> @# asd //모든 특수문자 프리픽스 다 받음.
!봇 시간  -> param0 = channel name , param 1 = !봇 시간
/msg somin aaaa bbbbb ccccccc -> msg =  aaaa bbbbb ccccccc
/DCC SEND somin a -> param 0 = [somin] , param 1 = DCC SEND a 2886795266 33227 0

성공적인 개인 메시지:
클라이언트 입력: PRIVMSG Alice :Hello, how are you?
서버 응답: (응답 없음, 메시지가 성공적으로 전달됨)
성공적인 채널 메시지:
클라이언트 입력: PRIVMSG #general :Hello everyone!
서버 응답: (응답 없음, 메시지가 성공적으로 전달됨)
존재하지 않는 닉네임:
클라이언트 입력: PRIVMSG NonExistentUser :Hello there!
서버 응답: :server 401 YourNick NonExistentUser :No such nick/channel
존재하지 않는 서버:
클라이언트 입력: PRIVMSG @NonExistentServer :Hello
서버 응답: :server 402 YourNick NonExistentServer :No such server
채널에 메시지를 보낼 수 없음 (예: 밴 상태):
클라이언트 입력: PRIVMSG #channel :Can't send this
서버 응답: :server 404 YourNick #channel :Cannot send to channel
너무 많은 대상:
클라이언트 입력: PRIVMSG Nick1,Nick2,Nick3,Nick4 :Hello all!
서버 응답: :server 407 YourNick :Too many recipients
수신자 없음:
클라이언트 입력: PRIVMSG
서버 응답: :server 411 YourNick :No recipient given (PRIVMSG)
보낼 텍스트 없음:
클라이언트 입력: PRIVMSG Alice
서버 응답: :server 412 YourNick :No text to send
자리 비움 상태의 사용자에게 메시지 전송:
클라이언트 입력: PRIVMSG AwayUser :Are you there?
서버 응답:
:server 301 YourNick AwayUser :I am away from my computer
(메시지는 여전히 전달됨)
채널 멤버십 접두사를 사용한 메시지:
클라이언트 입력: PRIVMSG @#channel :Message to ops
서버 응답: (응답 없음, 메시지가 채널 운영자에게만 전달됨)
브로드캐스트 메시지 (서버 간):
서버 간 메시지: :server PRIVMSG $*.net :Network announcement
클라이언트 수신: :server PRIVMSG $*.net :Network announcement
중재 모드 채널에서 발언 권한 없음:
클라이언트 입력: PRIVMSG #moderated :Can't speak here
서버 응답: :server 404 YourNick #moderated :Cannot send to channel
*/