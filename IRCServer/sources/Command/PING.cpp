#include "IRCServer.hpp"
#include "IRCContext.hpp"

#include <string>
#include <sstream>

#include "IRCResponseCreator.hpp"
#include "IRCClient.hpp"
#include "IRCErrors.hpp"

void IRCServer::ActionPING(IRCContext& context)
{
	if (context.params.size() != 1)
		throw IRCError::MissingParams();
	
	context.createSource = false;
  // PING의 응답은 PONG이므로 ConvertComToStr의 결과를 사용하지 않고 커맨드를 직접 작성
  // (ConvertComToStr에는 PING에 대응되는 값이 저장되어 있지 않음)
  context.numericResult = -1;
	std::stringstream ss;
	ss << "PONG " << _serverName << " :" << context.params[0];
	context.stringResult = ss.str();
	context.client->Send(IRC_response_creator::MakeResponse(context));
	context.pending_fds->insert(context.client->GetFD());
}
