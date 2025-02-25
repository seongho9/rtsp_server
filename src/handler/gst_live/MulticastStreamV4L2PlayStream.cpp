#include "handler/RTSPSessionHandlerLive.hpp"

#include "spdlog/spdlog.h"

int MulticastStreamV4L2::play_stream()
{
    std::thread live_stream([this](){
        gst_element_set_state(_rtp_info->pipeline, GST_STATE_PLAYING);
        //  pipeline 실행을 잡아두기 위한 GstBus 수행
        GstBus* bus = gst_element_get_bus(_rtp_info->pipeline);
        GstMessage* msg = nullptr;
        while(1) {
            msg = gst_bus_timed_pop_filtered(bus, 500 * GST_MSECOND, 
                static_cast<GstMessageType>(GST_MESSAGE_ERROR|GST_MESSAGE_EOS));
            if(msg != nullptr) {
                if(GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
                    spdlog::info("[MulticastStreamV4L2:play_stream] GstPipeline reach end of stream");
                    gst_message_unref(msg);
        
                    break;
                }
                else if(GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
                    GError* err = nullptr;
                    gchar* debug_info = nullptr;
                    gst_message_parse_error(msg, &err, &debug_info);
        
                    spdlog::error("[MulticastStreamV4L2:play_stream] GstPipeline Error {} {}", err->code, err->message);
                    g_free(debug_info);
                    g_clear_error(&err);
                    gst_message_unref(msg);
                        
                    break;
                }
                gst_message_unref(msg);
            }
        }
        gst_object_unref(bus);
        
        gst_element_set_state(_rtp_info->pipeline, GST_STATE_NULL);
        gst_object_unref(_rtp_info->pipeline);
    });
    live_stream.detach();

    return 0;
}
