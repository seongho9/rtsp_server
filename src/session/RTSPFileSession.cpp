
#include <spdlog/spdlog.h>

#include "session/RTSPFileSession.hpp"

namespace asio = boost::asio;
namespace hydra = boost::hydra;



RTSPFileSession* RTSPFileSession::build(
    boost::asio::io_context& ctx, boost::asio::ip::tcp::socket&& socket, 
    std::string uuid, std::unordered_map<std::string, std::string>& path)
{
    RTSPFileSessionGst* session = new RTSPFileSessionGst(ctx, std::move(socket), uuid, path);

    return static_cast<RTSPFileSession*>(session);
}