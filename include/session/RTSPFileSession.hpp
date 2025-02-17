
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

//  H.264 관련 메타정보
struct H264CodecInfo
{
    std::string encoding_name;
    std::string sprop_param;
    std::string framerate;
    std::string profile_level;
    uint32_t clock_rate = 0u;
    uint32_t ssrc = 0u;
    std::string packetization_mode;

    ~H264CodecInfo()
    {    }
};

struct DemuxStream
{
    void* video_stream;
    void* audio_stream;
};

//  RTP, RTCP 네트워크 정보
struct RTPSessionInfo
{
    boost::asio::ip::udp::socket*   rtp_socket;
    boost::asio::ip::udp::endpoint* rtp_endpoint;

    boost::asio::ip::udp::socket*   rtcp_socket;
    boost::asio::ip::udp::endpoint* rtcp_endpoint;

    uint16_t init_seq;
    uint32_t init_rtptime;

    ~RTPSessionInfo()
    {
        if(rtp_socket != nullptr)
            delete rtp_socket;
        
        if(rtp_endpoint != nullptr)
            delete rtp_endpoint;
        
        if(rtcp_socket != nullptr)
            delete rtcp_socket;

        if(rtcp_endpoint != nullptr)
            delete rtcp_endpoint;
    }
};

//  RTPSessionInfo의 Gstreamer형
struct RTPSessionInfoGst
{
    RTPSessionInfo* session_info = nullptr;

    GstElement* pipeline = nullptr;

    bool first = true;
    bool bus_enable = true;

    ~RTPSessionInfoGst()
    {
        if(session_info != nullptr){
            delete session_info;
        }
        gst_object_unref(pipeline);
    }
};

class RTSPFileSession
{

public:

    /// @brief RTSPSession 구현체를 가져오는 정적 메소드
    /// @param ctx boost asio의 io_context(비동기 처리를 위함)
    /// @param socket 해당 세션에 할당된 TCP 소켓
    /// @param uuid 세션 id
    /// @param path uri에 대응되는 영상의 위치
    /// @return RTSPession 구현체 포인터
    static RTSPFileSession* build(
        boost::asio::io_context& ctx, boost::asio::ip::tcp::socket&& socket, 
        std::string uuid, std::unordered_map<std::string, std::string>& path);


    /// @brief OPTION 요청 처리 메소드
    /// @param public_value public 헤더에 들어갈 값(반환 값)
    /// @return 0 : 성공, others: 실패
    virtual int option_request(std::string& public_value)=0;

    /// @brief DESCRIBE 요청 처리 메소드
    /// @param path 경로에 맞는 파일
    /// @param sdp 요청에 따른 sdp 값(반환 값)
    /// @return 0 : 성공, others: 실패
    virtual int describe_request(const std::string& path, std::string& sdp)=0;

    /// @brief SETUP 요청 처리 메소드
    /// @param path 경로에 맞는 파일
    /// @param client_addr 클라이언트의 ip 주소
    /// @param client_port 클라이언트가 RTP와 RTCP를 받아오려는 포트
    /// @param server_port 서버가 RTP와 RTCP를 전송하는 포트(반환 값)
    /// @return 0 : 성공, others : 실패
    virtual int setup_request(
        const std::string& path, 
        const std::string& client_addr, const std::string& client_port, 
        std::string& server_port
    )=0;

    /// @brief PLAY 요청 처리 메소드
    /// @param time 시작 시점(npt based)
    /// @param rtp_info 클라이언트에게 전송할 RTP 관련 정보(반환 값)
    /// @return 0 : 성공, others : 실패
    virtual int play_request(const std::string& time, std::string& rtp_info)=0;

    /// @brief PAUSE 요청 처리 메소드
    /// @param time 중지 시점(npt based)
    /// @return 0 : 성공, others : 실패
    virtual int pause_request(const std::string& time)=0;

    /// @brief TEARDOWN 요청 처리 메소드
    /// @return 0 : 성공, others : 실패
    virtual int teardown_request()=0;

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

class RTSPFileSessionGst
    :public RTSPFileSession
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

    bool _keep_alive = true;

    RTPSessionInfoGst* _rtp_info = nullptr;
    H264CodecInfo* _codec_info = nullptr;
    DemuxStream* _streams = nullptr;

public:
    RTSPFileSessionGst() = default;
    RTSPFileSessionGst(
        boost::asio::io_context& ctx, boost::asio::ip::tcp::socket&& socket, std::string uuid, 
        std::unordered_map<std::string, std::string>& path
    );

    ~RTSPFileSessionGst();

    int option_request(std::string& methods) override;

    int describe_request(const std::string& path, std::string& sdp) override;

    int setup_request(
        const std::string& path, 
        const std::string& client_addr, const std::string& client_port, std::string& server_port ) override;

    int play_request(const std::string& time, std::string& rtp_info) override;

    int pause_request(const std::string& time) override;

    int teardown_request() override;

    
    void destroy();

    void run() override;

    int read() override;

    boost::asio::ip::tcp::socket& socket() override;
};

#endif