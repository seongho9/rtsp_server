#include "handler/RTSPSessionHandlerLive.hpp"
#include <spdlog/spdlog.h>

RTSPSessionHandlerGstLive::RTSPSessionHandlerGstLive(boost::asio::io_context& context)
    :_context(context)
{   
    _handler = MulticastStream::get_instance("", 0);
}

RTSPSessionHandlerGstLive::~RTSPSessionHandlerGstLive()
{
    
}

int RTSPSessionHandlerGstLive::option_request(std::string& methods)
{
    methods = "DESCRIBE, OPTIONS, SETUP, PLAY, TEARDOWN, PAUSE";
    return 0;
}

int RTSPSessionHandlerGstLive::describe_request(const std::string& path, std::string& sdp)
{
    return _handler->make_sdp(sdp);
}

int RTSPSessionHandlerGstLive::setup_request(const std::string& path, const std::string& client_addr, const std::string& client_port, std::string& transport_header )
{
    std::string multicast_ip;
    _handler->get_ip_addr(multicast_ip);
    int multicast_port = _handler->get_port();
    transport_header.append("destination=").append(multicast_ip).append(";port=").append(std::to_string(multicast_port)).append(";ttl=32");
    
    return 0;
}

int RTSPSessionHandlerGstLive::play_request(const std::string& time, std::string& rtp_info)
{
    return 0;
}

int RTSPSessionHandlerGstLive::pause_request(const std::string& time)
{
    return 0;
}

int RTSPSessionHandlerGstLive::teardown_request()
{
    return 0;
}

int RTSPSessionHandlerGstLive::destroy()
{
    delete this;

    return 0;
}