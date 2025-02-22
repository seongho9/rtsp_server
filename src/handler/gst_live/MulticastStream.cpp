#include "handler/RTSPSessionHandlerLive.hpp"
#include <gst/app/app.h>
#include <spdlog/spdlog.h>
#include <fstream>

namespace asio = boost::asio;


MulticastStream* MulticastStream::_instance = nullptr;

MulticastStream* MulticastStream::get_instance(std::string multicast_ip, int port)
{
    if(_instance == nullptr) {
        _instance = static_cast<MulticastStream*> (new MulticastStreamV4L2(multicast_ip, port));
    }
    return _instance;
}