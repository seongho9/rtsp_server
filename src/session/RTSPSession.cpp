
#include "spdlog/spdlog.h"

#include "session/RTSPSession.hpp"

namespace asio = boost::asio;
namespace hydra = boost::hydra;



RTSPSession* RTSPSession::build(
    boost::asio::io_context& ctx, boost::asio::ip::tcp::socket&& socket, 
    std::string uuid, std::unordered_map<std::string, std::string>& path)
{
    RTSPSessionImpl* session = new RTSPSessionImpl(ctx, std::move(socket), uuid, path);

    return static_cast<RTSPSession*>(session);
}