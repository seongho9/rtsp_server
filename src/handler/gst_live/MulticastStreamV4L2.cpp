#include "handler/RTSPSessionHandlerLive.hpp"

#include <spdlog/spdlog.h>

MulticastStreamV4L2::MulticastStreamV4L2(std::string ip_addr, int port)   
    :_ip_addr(ip_addr), _port(port)
{ 
    _rtp_info = new RTPSessionInfoGst();
    _codec_info = new H264CodecInfo();
}

int MulticastStreamV4L2::destroy()
{
    return 0;
}

int MulticastStreamV4L2::get_ip_addr(std::string& ip_addr)
{
    ip_addr.assign(_ip_addr);

    return 0;
} 

int MulticastStreamV4L2::get_port()
{
    return _port;
}