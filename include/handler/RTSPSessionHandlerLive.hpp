#ifndef _RTSP_SESSION_HANDLER_LIVE_HPP
#define _RTSP_SESSION_HANDLER_LIVE_HPP

#include "handler/RTSPSessionHandler.hpp"


class MulticastStream
{
protected:
    static MulticastStream* _instance;
public:
    virtual int set_stream() = 0;
    virtual int make_sdp(std::string& sdp_str) = 0;
    virtual int play_stream() = 0;
    virtual int destroy() = 0;
    virtual int get_ip_addr(std::string& ip_addr) = 0;
    virtual int get_port() = 0;

    static MulticastStream* get_instance(std::string multicast_ip, int port);
};

class MulticastStreamV4L2 :public MulticastStream
{
private:
    RTPSessionInfoGst* _rtp_info;
    H264CodecInfo* _codec_info;
    bool _is_sdp = false;

    std::string _sdp_str;
    std::string _ip_addr;
    int         _port;

public:
    MulticastStreamV4L2() = default;
    MulticastStreamV4L2(std::string ip_addr, int port);

    int set_stream() override;
    int make_sdp(std::string& sdp_str) override;
    int play_stream() override;
    int destroy() override;  
    int get_ip_addr(std::string& ip_addr) override;
    int get_port() override;
}; 

class RTSPSessionHandlerGstLive : public RTSPSessionHandler
{
private:
        
    bool _keep_alive = true;

    boost::asio::io_context& _context;

    MulticastStream* _handler;

public:
    RTSPSessionHandlerGstLive() = delete;
    RTSPSessionHandlerGstLive(boost::asio::io_context& context);
    ~RTSPSessionHandlerGstLive();
    
    int option_request(std::string& public_value) override;
    int describe_request(const std::string& path, std::string& sdp) override;
    int setup_request(const std::string& path, 
                            const std::string& client_addr, const std::string& client_port, 
                            std::string& transport_header ) override;
    int play_request(const std::string& time, std::string& rtp_info) override;
    int pause_request(const std::string& time) override;
    int teardown_request() override;

    int destroy() override;
};

#endif