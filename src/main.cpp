#include "server/RTSPListener.hpp"
#include "handler/RTSPSessionHandlerLive.hpp"
#include <spdlog/spdlog.h>

std::string ip_addr("");

int main(int argc, char *argv[])
{
    if(argc != 2) {
        spdlog::info("{}", argc);
        spdlog::error("Usage : {} <ip_address>", argv[0]);
        return 1;
    }

    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("SET LOG LEVEL = DEBUG");

    ip_addr.assign(argv[1]);

    gst_init(&argc, &argv);

    const auto address = boost::asio::ip::make_address("0.0.0.0");
    int port_int = 8550;
    unsigned short port_s = static_cast<unsigned short>(port_int);


    
    try{
        boost::asio::io_context ioc{1};

                
        MulticastStream* handler = MulticastStream::get_instance("239.255.1.1", 5004);
        std::string sdp;
        handler->make_sdp(sdp);
        handler->set_stream();
        handler->play_stream();

        std::unordered_map<std::string, std::string> path;
        path.insert({"/test", "/home/seongho/sample.mp4"});
        path.insert({"/earth", "/home/seongho/earth.mp4"});
        path.insert({"/live", "/dev/video"});
    
        RTSPListener* listener = 
            new RTSPListenerImpl(ioc, boost::asio::ip::tcp::endpoint(address, port_s), path);
    
        listener->do_accept();
        ioc.run();
        
    } catch(std::exception& ex) {
        spdlog::error("ERRORERROR {}", ex.what());
    }


    return 0;
}
