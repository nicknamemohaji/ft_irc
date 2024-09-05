#include "IRCServer.hpp"
#include "IRCContext.hpp"

#include <string>
#include <sstream>

#include "IRCResponseCreator.hpp"
#include "IRCClient.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionPING(IRCContext& context)
{
  if (context.params.size() != 1) {
    return IRC_response_creator::ERR_NEEDMOREPARAMS(
        context.client, server_name_, context.pending_fds, context.command);
  }
	
	context.createSource = false;
  // PING의 응답은 PONG이므로 ConvertComToStr의 결과를 사용하지 않고 커맨드를 직접 작성
  context.command = UNKNOWN;
  context.numericResult = -1;
	std::stringstream ss;
	ss << "PONG " << server_name_ << " :" << context.params[0];
	context.stringResult = ss.str();
	context.client->Send(IRC_response_creator::MakeResponse(context));
	context.pending_fds->insert(context.client->GetFD());
}
