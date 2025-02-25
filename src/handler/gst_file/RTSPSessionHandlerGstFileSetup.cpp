#include <gst/app/app.h>
#include <gst/rtp/rtp.h>

#include <boost/asio/strand.hpp>

#include "spdlog/spdlog.h"

#include "handler/RTSPSessionHandler.hpp"

namespace asio = boost::asio;
namespace hydra = boost::hydra;

/// @brief qtdemux에서 audio 스트림과 video 스트림을 각각 element에 동적으로 연결하기 위한 콜백
/// @param src qtdemux
/// @param src_pad qtdemux에 동적으로 생성된 src pad
/// @param user_data 사용자 정의 변수로, 여기에서는 audio, video 스트림에 연결할 element를 가진 구조체
static void setup_demux_pad_added(GstElement* src, GstPad* src_pad, gpointer user_data);

/// @brief udp를 통해 rtp전송을 위한 appsink
/// @param appsink rtp_appsink element
/// @param user_data udp 소캣 관련 데이터
/// @return GST_FLOW_OK, GST_FLOW_ERROR
static GstFlowReturn rtp_appsink_new_sample_callback(GstAppSink* appsink, gpointer user_data);

/// @brief udp를 통해 rtcp전송을 위한 appsink
/// @param appsink rtp_appsink element
/// @param user_data udp 소캣 관련 데이터
/// @return GST_FLOW_OK, GST_FLOW_ERROR
static GstFlowReturn rtcp_appsink_new_sample_callback(GstAppSink* appsink, gpointer user_data);


int RTSPSessionHandlerGstFile::setup_request(
    const std::string& path, const std::string& client_addr, const std::string& client_port, std::string& transport_header )
{
    //  RTP 처리에 필요한 변수 선언
    if(_rtp_info != nullptr) {
        delete _rtp_info;
    }
    _rtp_info = new RTPSessionInfoGst();
    _rtp_info->session_info = new RTPSessionInfo();
    _streams = new DemuxStream();

    int rtp_port, rtcp_port;

    GstElement* pipeline = gst_pipeline_new("mp4_pipeline");

    //  mp4파일을 읽어옴
    GstElement* filesrc = gst_element_factory_make("filesrc", "filesrc");
    //  mp4파일 디먹싱(현 버전은 비디오만 처리)
    GstElement* demux = gst_element_factory_make("qtdemux", "demux");
    //  H.264 처리
    GstElement* video_q = gst_element_factory_make("queue", "video_q");
    _streams->video_stream = video_q;
    GstElement* h264parse = gst_element_factory_make("h264parse", "h264parse");
    GstElement* rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    //  rtp 통합 처리
    GstElement* rtpbin = gst_element_factory_make("rtpbin", "rtpbin");
    //  rtp 송신
    GstElement* rtp_appsink = gst_element_factory_make("appsink", "rtp_appsink");
    //  rtcp 송신
    GstElement* rtcp_appsink = gst_element_factory_make("appsink", "rtcp_appsink");
    //  rtcp 수신
    GstElement* rtcp_udpsrc = gst_element_factory_make("udpsrc", "rtcp_udpsrc");

    if(!pipeline || !filesrc || !demux || !video_q || !h264parse || !rtph264pay || !rtpbin || !rtp_appsink || !rtcp_appsink || !rtcp_udpsrc) {
        spdlog::error("[RTSPFileSessionGst:setup_request] Pipeline element creation failed");

        // 생성된 요소 정리
        if (pipeline)       gst_object_unref(pipeline);
        if (filesrc)        gst_object_unref(filesrc);
        if (demux)          gst_object_unref(demux);
        if (video_q)        gst_object_unref(video_q);
        if (h264parse)      gst_object_unref(h264parse);
        if (rtph264pay)     gst_object_unref(rtph264pay);
        if (rtpbin)         gst_object_unref(rtpbin);
        if (rtp_appsink)    gst_object_unref(rtp_appsink);
        if (rtcp_appsink)   gst_object_unref(rtcp_appsink);
        if (rtcp_udpsrc)    gst_object_unref(rtcp_udpsrc);

        delete _rtp_info;
        _rtp_info = nullptr;

        return 1;
    }

    //  클라이언트가 전송한 포트 번호를 int형으로 파싱
    sscanf(client_port.c_str(), "%d-%d", &rtp_port, &rtcp_port);

    //  filesrc에 읽어올 파일 경로 설정
    g_object_set(filesrc, "location", path.c_str(), NULL);
    //  rtp_appsink, rtcp_appsink 관련 설정
    g_object_set(rtp_appsink, "emit-signals", TRUE, NULL);
    g_object_set(rtcp_appsink, "emit-signals", TRUE, NULL);

    //  pipeline에 element 등록
    gst_bin_add_many(GST_BIN(pipeline),
        filesrc, demux, 
        video_q, h264parse, rtph264pay,
        rtpbin, rtp_appsink, rtcp_appsink, rtcp_udpsrc, NULL);
    
    //  element간 연결
    //  filesrc - qtdemux
    gst_element_link(filesrc, demux);
    //  qtdemux - streams(video_q, )
    g_signal_connect(demux, "pad-added", G_CALLBACK(setup_demux_pad_added), static_cast<void*>(_streams));
    //  video_q - h264parse
    gst_element_link(video_q, h264parse);
    //  h264parse - rtph264pay
    gst_element_link(h264parse, rtph264pay);
    //  rtph264pay - send_rtp_sink_0 of rtpbin
    GstPad* rtp_sink_pad = gst_element_get_request_pad(rtpbin, "send_rtp_sink_0");
    GstPad* pay_src_pad  = gst_element_get_static_pad(rtph264pay, "src");
    if(gst_pad_link(pay_src_pad, rtp_sink_pad) != GST_PAD_LINK_OK) {
        spdlog::error("[RTSPFileSessionGst:setup_request] failed to link rtph264pay src - rtpbin send_rtp_sink_0");
    }
    gst_object_unref(rtp_sink_pad);
    gst_object_unref(pay_src_pad);
    //  send_rtp_src_0 of rtpbin - rtp_appsink
    GstPad* rtp_src_pad = gst_element_get_static_pad(rtpbin, "send_rtp_src_0");
    GstPad* rtp_appsink_pad = gst_element_get_static_pad(rtp_appsink, "sink");
    if(gst_pad_link(rtp_src_pad, rtp_appsink_pad) != GST_PAD_LINK_OK) {
        spdlog::error("[RTSPFileSessionGst:setup_request] failed to link rtpbin send_rtp_src_0 - rtp_appsink");
    }
    gst_object_unref(rtp_src_pad);
    gst_object_unref(rtp_appsink_pad);
    //  rtcp_udpsrc - recv_rtcp_sink_0 of rtpbin
    GstPad* recv_rtcp_sink_pad = gst_element_get_request_pad(rtpbin, "recv_rtcp_sink_0");
    GstPad* udp_src_pad = gst_element_get_static_pad(rtcp_udpsrc, "src");
    if(gst_pad_link(udp_src_pad, recv_rtcp_sink_pad) != GST_PAD_LINK_OK) {
        spdlog::error("[RTSPFileSessionGst:setup_request] failed to link udpsrc - rtpbin recv_rtcp_sink_0");
    }
    gst_object_unref(recv_rtcp_sink_pad);
    gst_object_unref(udp_src_pad);
    //  send_rtcp_src_0 of rtpbin - rtcp_appsink
    GstPad* send_rtcp_src_pad = gst_element_get_request_pad(rtpbin, "send_rtcp_src_0");
    GstPad* rtcp_appsink_pad = gst_element_get_static_pad(rtcp_appsink, "sink");
    if(gst_pad_link(send_rtcp_src_pad, rtcp_appsink_pad) != GST_PAD_LINK_OK) {
        spdlog::error("[RTSPFileSessionGst:setup_request] failed to link rtpbin send_rtcp_src_0 - rtcp_appsink");
    }
    gst_object_unref(send_rtcp_src_pad);
    gst_object_unref(rtcp_appsink_pad);

    //  송신 소캣을 할당 : 할당된 포트 번호를 알아오기 위함
    //  send rtp socket
    _rtp_info->session_info->rtp_socket = new asio::ip::udp::socket(asio::make_strand(_context));
    _rtp_info->session_info->rtp_endpoint = new asio::ip::udp::endpoint(asio::ip::address::from_string(client_addr), rtp_port);
    _rtp_info->session_info->rtp_socket->open(asio::ip::udp::v4());
    _rtp_info->session_info->rtp_socket->bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
    g_signal_connect(rtp_appsink, "new-sample", G_CALLBACK(rtp_appsink_new_sample_callback), _rtp_info);
    //  send rtcp socket
    _rtp_info->session_info->rtcp_socket = new asio::ip::udp::socket(asio::make_strand(_context));
    _rtp_info->session_info->rtcp_endpoint = new asio::ip::udp::endpoint(asio::ip::address::from_string(client_addr), rtcp_port);
    _rtp_info->session_info->rtcp_socket->open(asio::ip::udp::v4());
    g_signal_connect(rtcp_appsink, "new-sample", G_CALLBACK(rtcp_appsink_new_sample_callback), _rtp_info);

    asio::ip::udp::endpoint local_endpoint = _rtp_info->session_info->rtp_socket->local_endpoint();
    //  rtcp 수신 포트번호 설정
    g_object_set(rtcp_udpsrc, "port", local_endpoint.port() + 1, NULL);

    transport_header.append("server_port=");
    transport_header.append(std::to_string(local_endpoint.port()));
    transport_header.append("-");
    transport_header.append(std::to_string(local_endpoint.port()+1));

    //  pipeline 상태 변경
    GstStateChangeReturn ret;

    ret = gst_element_set_state(pipeline, GST_STATE_PAUSED);
    if(ret == GST_STATE_CHANGE_FAILURE) {
        spdlog::error("[RTSPFileSessionGst:setup_request] Failed to set pipeline state to PAUSED");
        gst_object_unref(pipeline);
        return 1;
    }
    _rtp_info->pipeline = pipeline;
    std::thread bus_thread(
        [pipeline](){
            spdlog::debug("RTP handler GstBus started");

            GstBus* bus = gst_element_get_bus(pipeline);
            GstMessage* msg;
            while(true) {  
        
                GstStateChangeReturn ret;
                ret = gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
                if(ret == GST_STATE_CHANGE_FAILURE){
                    break;
                }
                
                msg = gst_bus_timed_pop_filtered(bus, 500 * GST_MSECOND, 
                    static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

                if(msg != NULL) {
                    if(GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
                        spdlog::info("[RTSPFileSessionGst:setup_request] media stream end");
                        gst_message_unref(msg);
                        break;
                    }
                    else if(GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
                        GError* err;
                        gchar* debug_info;
        
                        gst_message_parse_error(msg, &err, &debug_info);
                        spdlog::error("[RTSPFileSessionGst:setup_request] Gstreamer Bus Error {}", err->message);

                        g_free(debug_info);
                        g_clear_error(&err);
                        gst_message_unref(msg);
                        break;
                    }
                    gst_message_unref(msg);
                
                }
            }
            spdlog::debug("[RTSPFileSessionGst:setup_request] RTP handler GstBus ended");
            gst_object_unref(bus);
        });
    bus_thread.detach();

    return 0;
}

void setup_demux_pad_added(GstElement* src, GstPad* src_pad, gpointer user_data)
{
    GstCaps* src_caps = gst_pad_get_current_caps(src_pad);
    if(!src_caps){
        spdlog::error("[RTSPFileSessionGst:setup_request::setup_demux_pad_added] failed to get GstCaps");
        return;
    }

    DemuxStream* streams = static_cast<DemuxStream*>(user_data);

    GstStructure* src_caps_structure = gst_caps_get_structure(src_caps, 0);
    const gchar* type = gst_structure_get_name(src_caps_structure);

    //  video 코덱이 H.264인 경우
    if(g_str_has_prefix(type, "video/x-h264")) {
        //  streams에서 video의 stream객체를 가져온다
        GstElement* video_element = static_cast<GstElement*>(streams->video_stream);

        //  이후 요소에서 sink 패드를 가져와 demux의 src와 연결
        GstPad* video_sink_pad = gst_element_get_static_pad(video_element, "sink");
        if(video_sink_pad == NULL){
            spdlog::error("[RTSPFileSessionGst:setup_request::setup_demux_pad_added] failed to getsink pad of video stream");
            gst_caps_unref(src_caps);
            return;
        }

        if(gst_pad_link(src_pad, video_sink_pad) != GST_PAD_LINK_OK) {
            spdlog::error("[RTSPFileSessionGst:setup_request::setup_demux_pad_added] failed link qtdemux and video stream");
        }
        gst_caps_unref(src_caps);
        gst_object_unref(video_sink_pad);
    }
}

GstFlowReturn rtp_appsink_new_sample_callback(GstAppSink* appsink, gpointer user_data)
{
    RTPSessionInfoGst* sess_info = static_cast<RTPSessionInfoGst*>(user_data);

    //  appsink에서 샘플을 받아옴
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(appsink));
    if(!sample) {
        spdlog::error("[RTSPFileSessionGst:setup_request::rtp_appsink_new_sample_callback] failed to pull sample from appsink");
        return GST_FLOW_ERROR;
    }

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstMapInfo map_info;

    //  sequence번호, rtptime을 받아옴 (초기 1번)
    if(sess_info->first) {

        GstRTPBuffer rtp = GST_RTP_BUFFER_INIT;
        if(!gst_rtp_buffer_map(buffer, GST_MAP_READ, &rtp)) {
            spdlog::error("[RTSPFileSessionGst:setup_request::rtp_appsink_new_sample_callback] failed to get RTPBuffer");
            gst_sample_unref(sample);
            return GST_FLOW_ERROR;
        }

        guint16 seq = gst_rtp_buffer_get_seq(&rtp);
        guint32 time = gst_rtp_buffer_get_timestamp(&rtp);

        sess_info->session_info->init_seq = static_cast<uint16_t>(seq);
        sess_info->session_info->init_rtptime = static_cast<uint32_t>(time);

        sess_info->first = false;

        gst_rtp_buffer_unmap(&rtp);
    }

    if(!gst_buffer_map(buffer, &map_info, GST_MAP_READ)) {
        spdlog::error("[RTSPFileSessionGst:setup_request::rtp_appsink_new_sample_callback] failed to get GstMapInfo");
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    //  udp로 전송
    boost::system::error_code ec;
    asio::ip::udp::socket* socket = sess_info->session_info->rtp_socket;
    if(socket->is_open()) {
        socket->send_to(
            asio::buffer(static_cast<void*>(map_info.data), map_info.size),
            *(sess_info->session_info->rtp_endpoint),
            0, ec);
        if(ec.failed()) {
            spdlog::error("[RTSPFileSessionGst:setup_request::rtp_appsink_new_sample_callback] failed to send RTP packet");
        }
        gst_buffer_unmap(buffer, &map_info);
    }

    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

GstFlowReturn rtcp_appsink_new_sample_callback(GstAppSink* appsink, gpointer user_data)
{
    RTPSessionInfoGst* sess_info = static_cast<RTPSessionInfoGst*>(user_data);

    //  appsink에서 샘플을 받아옴
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(appsink));
    if(!sample) {
        spdlog::error("[RTSPFileSessionGst:setup_request::rtcp_appsink_new_sample_callback] failed to pull sample from appsink");
        return GST_FLOW_ERROR;
    }

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstMapInfo map_info;


    if(!gst_buffer_map(buffer, &map_info, GST_MAP_READ)) {
        spdlog::error("[RTSPFileSessionGst:setup_request::rtcp_appsink_new_sample_callback] failed to get GstMapInfo");
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    //  udp로 전송
    boost::system::error_code ec;
    asio::ip::udp::socket* socket = sess_info->session_info->rtcp_socket;
    if(socket->is_open()) {
        socket->send_to(
            asio::buffer(static_cast<void*>(map_info.data), map_info.size),
            *(sess_info->session_info->rtcp_endpoint),
            0, ec);
        if(ec.failed()) {
            spdlog::error("[RTSPFileSessionGst:setup_request::rtcp_appsink_new_sample_callback] failed to send RTP packet");
        }
    }

    gst_buffer_unmap(buffer, &map_info);
    gst_sample_unref(sample);   

    return GST_FLOW_OK;
}