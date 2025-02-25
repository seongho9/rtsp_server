
#include "server/RTSPListener.hpp"

#include "spdlog/spdlog.h"
#include <memory>

RTSPListenerImpl::RTSPListenerImpl(boost::asio::io_context& ctx, boost::asio::ip::tcp::endpoint endpoint, std::unordered_map<std::string, std::string> path)
    : _context(ctx), _acceptor(boost::asio::make_strand(ctx)), _path(path)
{
    boost::system::error_code ec;
    
    _acceptor.open(endpoint.protocol(), ec);
    if(ec.failed()) {
        spdlog::error("Failed to open RTSP acceptor : {}", ec.message());
    }
    _acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if(ec.failed()) {
        spdlog::error("Failed to set option address reuse : {}", ec.message());
    }


    _acceptor.bind(endpoint, ec);
    _acceptor.listen(boost::asio::socket_base::max_connections, ec);
    if(ec.failed()) {
        spdlog::error("Failed to listen RTSP acceptor : {}", ec.message());
    }

    spdlog::info("Server Open to {}:{}", endpoint.address().to_string(), endpoint.port());
}

void RTSPListenerImpl::do_accept()
{
    _acceptor.async_accept(
        boost::asio::make_strand(_context),
        [self=this](boost::hydra::error_code ec, boost::asio::ip::tcp::socket socket)
        {
            if (ec.failed()) {
                spdlog::error("Async accept failed: {}", ec.message());
            } 
            else {
                
                std::thread sess_thread(&RTSPListenerImpl::handle_session, self, std::move(socket));

                sess_thread.detach();
            }
            
            self->do_accept();
        }
    );

}

void RTSPListenerImpl::handle_session(boost::asio::ip::tcp::socket&& socket)
{

    std::string sess_id = boost::uuids::to_string(_generator());
    spdlog::debug("session id {}", sess_id);

    RTSPSession* sess_obj = RTSPSession::build(_context, std::move(socket), sess_id, _path);

    sess_obj->run();

    sess_obj->destroy();
    
    spdlog::info("[RTSPListenerImpl:handler_session] delete session id - {}", sess_id);
}