#ifndef _RTSP_SESSION_HANDLER_HPP
#define _RTSP_SESSION_HANDLER_HPP

#include <boost/asio/ip/udp.hpp>
#include <boost/asio.hpp>

#include <gst/gst.h>

#include <boost/hydra/core.hpp>
#include <boost/hydra/rtsp.hpp>

#include <string>

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

//  비디오, 오디오 스트림 정보
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

class RTSPSessionHandler
{
public:
    
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
    virtual int setup_request(const std::string& path, 
                            const std::string& client_addr, const std::string& client_port, 
                            std::string& transport_header )=0;

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

    /// @brief 객체 삭제(구현객체를 없애기 위함)
    /// @return 0 성공
    virtual int destroy() = 0;
};

class RTSPSessionHandlerGstFile : public RTSPSessionHandler
{
private:
        
    bool _keep_alive = true;

    RTPSessionInfoGst* _rtp_info = nullptr;
    H264CodecInfo* _codec_info = nullptr;
    DemuxStream* _streams = nullptr;

    boost::asio::io_context& _context;

public:
    RTSPSessionHandlerGstFile() = delete;
    RTSPSessionHandlerGstFile(boost::asio::io_context& context);
    ~RTSPSessionHandlerGstFile();
    
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