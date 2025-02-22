
#include <spdlog/spdlog.h>

#include "handler/RTSPSessionHandler.hpp"

int RTSPSessionHandlerGstFile::play_request(const std::string& time, std::string& rtp_info)
{

    GstStateChangeReturn ret = gst_element_set_state(_rtp_info->pipeline, GST_STATE_PLAYING);
    if(ret == GST_STATE_CHANGE_FAILURE) {
        spdlog::error("[RTSPFileSessionGst:play_request] Failed to set pipeline state to PLAYING");
        return 1;
    }


    //  첫 패킷에서 정보를 가져올 때 까지 busy waiting으로 실행의 흐름을 잡아둠
    while(_rtp_info->first);

    rtp_info.assign("seq=");
    rtp_info.append(std::to_string(_rtp_info->session_info->init_seq));

    rtp_info.append(";rtptime=");
    rtp_info.append(std::to_string(_rtp_info->session_info->init_rtptime));

    return 0;
}