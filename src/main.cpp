#include "server/RTSPListener.hpp"
#include <spdlog/spdlog.h>

std::string ip_addr("");

int main(int argc, char *argv[])
{
    if(argc != 2) {
        spdlog::info("{}", argc);
        spdlog::error("Usage : {} <ip_address>", argv[0]);
        return 1;
    }

    spdlog::set_level(spdlog::level::info);

    ip_addr.assign(argv[1]);

    gst_init(&argc, &argv);

    const auto address = boost::asio::ip::make_address("0.0.0.0");
    int port_int = 8550;
    unsigned short port_s = static_cast<unsigned short>(port_int);
    
    try{
        boost::asio::io_context ioc{1};

        std::unordered_map<std::string, std::string> path;
        path.insert({"/test", "/home/seongho/sample.mp4"});
    
        RTSPListener* listener = 
            new RTSPListenerImpl(ioc, boost::asio::ip::tcp::endpoint(address, port_s), path);
    
        listener->do_accept();
        ioc.run();
        
    } catch(std::exception& ex) {
        spdlog::error("ERRORERROR {}", ex.what());
    }

    return 0;
}
