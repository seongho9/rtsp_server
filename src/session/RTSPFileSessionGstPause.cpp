
#include <spdlog/spdlog.h>

#include "session/RTSPFileSession.hpp"

int RTSPFileSessionGst::pause_request(const std::string& time)
{

    GstStateChangeReturn ret = gst_element_set_state(_rtp_info->pipeline, GST_STATE_PAUSED);
    if(ret == GST_STATE_CHANGE_FAILURE) {
        spdlog::error("[RTSPFileSessionGst:play_request] Failed to set pipeline state to PLAYING");
        return 1;
    }
    
    return 0;
}