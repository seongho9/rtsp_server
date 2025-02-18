
#ifndef _RTSP_SESSION_HPP
#define _RTSP_SESSION_HPP

#include <string>
#include <unordered_map>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>

#include <gst/gst.h>

#include <boost/hydra/core.hpp>
#include <boost/hydra/rtsp.hpp>

#include "handler/RTSPSessionHandler.hpp"

class RTSPSession
{

public:

    /// @brief RTSPSession 구현체를 가져오는 정적 메소드
    /// @param ctx boost asio의 io_context(비동기 처리를 위함)
    /// @param socket 해당 세션에 할당된 TCP 소켓
    /// @param uuid 세션 id
    /// @param path uri에 대응되는 영상의 위치
    /// @return RTSPession 구현체 포인터
    static RTSPSession* build(
        boost::asio::io_context& ctx, boost::asio::ip::tcp::socket&& socket, 
        std::string uuid, std::unordered_map<std::string, std::string>& path);

    /// @brief 초기 TCP 세션 생성시 진입점
    virtual void run()=0;

    ///  @brief 비동기 읽기 콜백 함수
    ///  @return 0: 세션을 유지, others : 세션 끊음
    virtual int read()=0;

    /// @brief 소켓 획득
    virtual boost::asio::ip::tcp::socket& socket() = 0;

    /// @brief 객체 본인을 삭제, 이 때, 해제 해야하는 자원을 위해 구현체에서 구현
    virtual void destroy() = 0;

};

class RTSPSessionImpl
    :public RTSPSession
{
private:
    std::string _uuid;

    boost::asio::io_context& _context;
    boost::asio::ip::tcp::socket& _socket;

    std::unordered_map<std::string, std::string>& _path;

    boost::asio::streambuf _buffer;
    boost::hydra::rtsp::request<boost::hydra::rtsp::string_body> _request;

    std::string _url_path;
    std::string _file_path;


    RTSPSessionHandler* _handler = nullptr;

public:
    RTSPSessionImpl() = default;
    RTSPSessionImpl(
        boost::asio::io_context& ctx, boost::asio::ip::tcp::socket&& socket, std::string uuid, 
        std::unordered_map<std::string, std::string>& path
    );

    ~RTSPSessionImpl();

    void destroy();
    void run() override;
    int read() override;
    
    boost::asio::ip::tcp::socket& socket() override;
};

#endif