#include <fstream>

#include <spdlog/spdlog.h>

#include <gst/sdp/gstsdpmessage.h>
#include <gst/app/app.h>

#include "handler/RTSPSessionHandler.hpp"


namespace asio = boost::asio;
namespace hydra = boost::hydra;

extern std::string ip_addr;

static GstFlowReturn new_sample_callback(GstAppSink *appsink, gpointer user_data);

static void demux_pad_added(GstElement* src, GstPad* pad, gpointer user_data);

int RTSPSessionHandlerGstFile::describe_request(const std::string& path, std::string& sdp)
{

    std::string sdp_path = path.substr(0, path.find(".")).append(".sdp");

    //  sdp파일이 존재하는지 확인
    std::ifstream sdp_file(sdp_path);

    //  존재하면 읽어서 리턴
    if(sdp_file.good()) {
        std::ostringstream sdp_stream;
        sdp_stream << sdp_file.rdbuf();

        sdp.assign(sdp_stream.str());

        return 0;
    }

    //  존재하지 않으면 파이프라인 생성 후 반환
    GstElement* pipeline = gst_pipeline_new("sdp-pipeline");
    //  mp4파일을 읽어옴
    GstElement* filesrc = gst_element_factory_make("filesrc", "filesrc");

    //  mp4파일 디먹싱(현 버전은 비디오만 처리)
    GstElement* demux = gst_element_factory_make("qtdemux", "demux");

    //  H.264 파일 처리
    GstElement* video_q = gst_element_factory_make("queue", "video_q");
    GstElement* h264parse = gst_element_factory_make("h264parse", "h264parse");
    GstElement* rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");

    //  rtp payload로 부터 sdp 관련 정보 읽어옴
    GstElement* appsink = gst_element_factory_make("appsink", "appsink");

    _codec_info = new H264CodecInfo();

    if(!pipeline || !filesrc || !demux || !video_q || !h264parse || !rtph264pay || !appsink) {
        spdlog::error("[RTSPFileSessionGst:describe_request] Pipeline element creation failed");

        // 생성된 요소 정리
        if (pipeline)   gst_object_unref(pipeline);
        if (filesrc)    gst_object_unref(filesrc);
        if (demux)      gst_object_unref(demux);
        if (video_q)    gst_object_unref(video_q);
        if (h264parse)  gst_object_unref(h264parse);
        if (rtph264pay) gst_object_unref(rtph264pay);
        if (appsink)    gst_object_unref(appsink);

        delete _codec_info;
        _codec_info = nullptr;

        return 1;
    }
    //  filesrc에 읽어올 파일 경로 설정
    g_object_set(filesrc, "location", path.c_str(), NULL);

    //  sdp관련 정보를 읽어올 콜백 설정
    g_object_set(appsink, "emit-signals", TRUE, NULL);
    g_signal_connect(appsink, "new-sample", G_CALLBACK(new_sample_callback), static_cast<gpointer>(_codec_info));

    //  element 간 연결
    gst_bin_add_many(GST_BIN(pipeline), filesrc, demux, video_q, h264parse, rtph264pay, appsink, NULL);
    //  filesrc - qtdemux
    gst_element_link(filesrc, demux);
    //  qtdemux - streams(video_q, )
    DemuxStream demux_stream;
    demux_stream.video_stream = video_q;
    demux_stream.audio_stream = nullptr;
    g_signal_connect(demux, "pad-added", G_CALLBACK(demux_pad_added), static_cast<void*>(&demux_stream));
    //  video_q - h264parse
    gst_element_link(video_q, h264parse);
    //  h264parse - rtph264pay
    gst_element_link(h264parse, rtph264pay);
    //  rtph264pay - appsink
    gst_element_link(rtph264pay, appsink);

    //  pipeline 실행
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    //  pipeline 실행을 잡아두기 위한 GstBus 수행
    GstBus* bus = gst_element_get_bus(pipeline);
    GstMessage* msg = nullptr;
    while(1) {
        msg = gst_bus_timed_pop_filtered(bus, 500 * GST_MSECOND, 
            static_cast<GstMessageType>(GST_MESSAGE_ERROR|GST_MESSAGE_EOS));
        if(msg != nullptr) {
            if(GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
                spdlog::info("[RTSPFileSessionGst:describe_request] GstPipeline reach end of stream");
                gst_message_unref(msg);

                break;
            }
            else if(GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
                GError* err = nullptr;
                gchar* debug_info = nullptr;
                gst_message_parse_error(msg, &err, &debug_info);

                spdlog::error("[RTSPFileSessionGst:describe_request] GstPipeline Error {}", err->message);

                g_free(debug_info);
                g_clear_error(&err);
                gst_message_unref(msg);
                
                break;
            }
            gst_message_unref(msg);
        }
    }
    gst_object_unref(bus);

    gst_object_unref(pipeline);

    //  파이프라인 실행 이후 구조체의 정보를 이용 SDP 파일을 생성
    GstSDPMessage* sdp_msg = nullptr;
    gst_sdp_message_new(&sdp_msg);

    //  v=0
    gst_sdp_message_set_version(sdp_msg, 0);
    //  o= - <sess_id> <sess_ver> <net_type> <addr_type> <adddr>
    //  시간 기반 sess_id
    uint64_t sess_id = static_cast<uint64_t>(time(NULL));
    uint64_t sess_ver = 0;
    gst_sdp_message_set_origin(sdp_msg, 
        "-", std::to_string(sess_id).c_str(), std::to_string(sess_ver).c_str(),
        "IN", "IP4", ip_addr.c_str());
    //  s= <sess_name>
    gst_sdp_message_set_session_name(sdp_msg, "Hydra RTSP Session");
    //  t=
    //  생략.. t= 0 0

    //  m= & a= 
    GstSDPMedia* media;
    gst_sdp_media_new(&media);

    //  m= <media> <port> <proto> <format>
    gst_sdp_media_set_media(media, "video");
    gst_sdp_media_set_port_info(media, 0, 1);
    gst_sdp_media_set_proto(media, "RTP/AVP");
    //  format
    if(_codec_info->encoding_name == "H264") {
        gst_sdp_media_add_format(media,"96");

        //  a=fmtp:<format> packetization-mode=*;sprop-parameter-sets=<SPS,PPS>;profile-level-id=<profile>;
        std::string fmtp_value;
        fmtp_value.assign("96");
        fmtp_value.append(" packetization-mode=");
        fmtp_value.append(_codec_info->packetization_mode);
        fmtp_value.append(";sprop-parameter-sets=");
        fmtp_value.append(_codec_info->sprop_param);
        fmtp_value.append(";profile-level-id=");
        fmtp_value.append(_codec_info->profile_level);
        gst_sdp_media_add_attribute(media, "fmtp", fmtp_value.c_str());

        //  a=rtpmap:96 H264/90000 
        std::string rtpmap_value;
        rtpmap_value.assign("96 ");
        rtpmap_value.append(_codec_info->encoding_name);
        rtpmap_value.append("/");
        rtpmap_value.append(std::to_string(_codec_info->clock_rate));
        gst_sdp_media_add_attribute(media, "rtpmap", rtpmap_value.c_str());
    }

    //  a=framerate:<fps>
    gst_sdp_media_add_attribute(media, "framerate", _codec_info->framerate.c_str());

    //  설정한 media 테그 정보를 sdp에 추가
    gst_sdp_message_add_media(sdp_msg, media);

    //  string으로 변환 및 파일로 저장
    sdp.assign(gst_sdp_message_as_text(sdp_msg));

    spdlog::debug("SDP description \n {}", sdp);
    //  sdp 저장을 위한 파일 스트림 생성
    std::ofstream sdp_outfile(sdp_path);
    if(!sdp_outfile){
        spdlog::error("[RTSPFileSessionGst:describe_request] Failed to Save file. path : {}", sdp_path);
        return 2;
    }
    sdp_outfile << sdp;
    sdp_outfile.close();

    return 0;
}

GstFlowReturn new_sample_callback(GstAppSink *appsink, gpointer user_data)
{
    if(user_data == nullptr) {
        spdlog::error("[RTSPFileSessionGst:describe_request:new_sample_callback] user_data is null");
        return GST_FLOW_ERROR;
    }
    H264CodecInfo* codec_info = static_cast<H264CodecInfo*>(user_data);

    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if (!sample) {
        spdlog::error("[RTSPFileSessionGst:describe_request:new_sample_callback] Failed to pull sample from appsink");
        return GST_FLOW_ERROR;
    }

    GstCaps* caps =gst_sample_get_caps(sample);
    if(!caps) {
        spdlog::error("[RTSPFileSessionGst:describe_request:new_sample_callback] Failed to get caps from sample");
    }

    GstStructure* structure = gst_caps_get_structure(caps, 0);
    if(!structure) {
        spdlog::error("[RTSPFileSessionGst:describe_request:new_sample_callback] Failed to get structure from sample");
    }
    
    if (gst_structure_has_field(structure, "encoding-name")) {
        const gchar* encoding_name = gst_structure_get_string(structure, "encoding-name");
        
        if(encoding_name) {
            codec_info->encoding_name.assign(encoding_name);
        }
        else {
            spdlog::warn("[RTSPFileSessionGst:describe_request:new_sample_callback] encoding_name is null");
        }

    }

    if (gst_structure_has_field(structure, "sprop-parameter-sets")) {
        const gchar* sprop_param = nullptr;
        sprop_param = gst_structure_get_string(structure, "sprop-parameter-sets");

        if(sprop_param) {
            codec_info->sprop_param.assign(sprop_param);
        }
        else {
            spdlog::warn("[RTSPFileSessionGst:describe_request:new_sample_callback] sprop_parameter-sets is null");
        }
    }

    if (gst_structure_has_field(structure, "clock-rate")) {
        gint c_rate;
        if(gst_structure_get_int(structure, "clock-rate", &c_rate)){
            codec_info->clock_rate = static_cast<uint32_t>(c_rate);
        }
        else{
            spdlog::warn("[RTSPFileSessionGst:describe_request:new_sample_callback] Failed to get clock-rate");
        }
    }

    if (gst_structure_has_field(structure, "ssrc")) {
        guint ssrc;
        if(gst_structure_get_uint(structure, "ssrc", &ssrc)) {
            codec_info->ssrc = static_cast<uint32_t>(ssrc);
        }
        else{
            spdlog::warn("[RTSPFileSessionGst:describe_request:new_sample_callback] Failed to get ssrc");
        }
    }

    if (gst_structure_has_field(structure, "a-framerate")) {
        const gchar* framerate = nullptr;
        framerate = gst_structure_get_string(structure, "a-framerate");

        if(framerate){
            codec_info->framerate.assign(framerate);
        }
        else{
            spdlog::warn("[RTSPFileSessionGst:describe_request:new_sample_callback] a-framerate is null");
        }
    }

    if (gst_structure_has_field(structure, "profile-level-id")) {
        const gchar* profile_level = nullptr;
        profile_level = gst_structure_get_string(structure, "a-framerate");

        if(profile_level){
            codec_info->profile_level.assign(profile_level);
        }
        else{
            spdlog::warn("[RTSPFileSessionGst:describe_request:new_sample_callback] profile-level-id is null");
        }
    }

    if (gst_structure_has_field(structure, "packetization-mode")) {
        const gchar* packetization = nullptr;
        packetization = g_strdup(gst_structure_get_string(structure, "packetization-mode"));

        if(packetization){
            codec_info->packetization_mode.assign(packetization);
        }
        else{
            spdlog::warn("[RTSPFileSessionGst:describe_request:new_sample_callback] packetization-mode is null");
        }
    }

    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

void demux_pad_added(GstElement* src, GstPad* pad, gpointer user_data)
{
    GstCaps* pad_caps = gst_pad_query_caps(pad, NULL);
    if(!pad_caps) {
        spdlog::error("[RTSPFileSessionGst:describe_request:demux_pad_add] failed to get GstCaps");
        return;
    }
    DemuxStream* streams = static_cast<DemuxStream*>(user_data);

    GstStructure* caps_struct = gst_caps_get_structure(pad_caps, 0);
    const gchar* type = gst_structure_get_name(caps_struct);
    
    spdlog::debug("[RTSPFileSessionGst:describe_request:demux_pad_add] {}", type);
    //  video의 코덱이 H.264인 경우
    if(g_str_has_prefix(type, "video/x-h264")) {
        spdlog::debug("[RTSPFileSessionGst:describe_request:demux_pad_add] H.264");
        //  streams에서 video의 stream객체를 가져온다
        GstElement* video_element = static_cast<GstElement*>(streams->video_stream);

        //  이후 요소에서 sink 패드를 가져와 demux의 src와 연결
        GstPad* video_sink_pad = gst_element_get_static_pad(video_element, "sink");
        if(video_sink_pad == NULL){
            spdlog::error("[RTSPFileSessionGst:describe_request:demux_pad_add] failed to getsink pad of video stream");
            gst_caps_unref(pad_caps);
            return;
        }

        if(gst_pad_link(pad, video_sink_pad) != GST_PAD_LINK_OK) {
            spdlog::error("[RTSPFileSessionGst:describe_request:demux_pad_add] failed link qtdemux and video stream");
        }

        gst_object_unref(video_sink_pad);
        gst_caps_unref(pad_caps);
    }

}