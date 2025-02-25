#include "handler/RTSPSessionHandlerLive.hpp"
#include <gst/gst.h>
#include "spdlog/spdlog.h"

int MulticastStreamV4L2::set_stream()
{
    GstElement* pipeline = gst_pipeline_new("streaming_pipeline");
    // src
    GstElement* v4l2_src = gst_element_factory_make("v4l2src", "source");
    g_object_set(v4l2_src, "device", "/dev/video0", NULL);
    //  capsfilter
    GstElement* caps_filter = gst_element_factory_make("capsfilter", "capsfilter");
    GstCaps* caps = gst_caps_new_simple("video/x-raw", 
        "format", G_TYPE_STRING, "YUY2",
        "width", G_TYPE_INT, 640, "height", G_TYPE_INT, 480,
        "framerate", GST_TYPE_FRACTION, 30, 1,
        NULL);
    g_object_set(caps_filter, "caps", caps, NULL);
    gst_caps_unref(caps);

    GstElement* videoconvert = gst_element_factory_make("videoconvert", "convert");

    GstElement* x264enc = gst_element_factory_make("x264enc", "encoder");
    g_object_set(x264enc, 
        "speed-preset", 1, //  0: none, 1: ultrafast, 2: superfast, 등
        "tune", 1, //   0: none, 1: fastdecode, 2: zerolatency, ...
        NULL);

    GstElement* rtph264pay = gst_element_factory_make("rtph264pay", "payloader");
    g_object_set(rtph264pay, "config-interval", 1, "pt", 96, NULL);

    GstElement* sink = gst_element_factory_make("udpsink", "rtp_sink");
    g_object_set(sink, "host", _ip_addr.c_str(), "port", _port, NULL);

    // 요소들을 링크
    gst_bin_add_many(GST_BIN(pipeline), v4l2_src, caps_filter, videoconvert, x264enc, rtph264pay, sink, NULL);
    if (!gst_element_link_many(v4l2_src, caps_filter, videoconvert, x264enc, rtph264pay, sink, NULL)) {
        spdlog::error("[MulticastStreamV4L2:set_stream] Failed to link elements");
        return -1;
    }
    
    gst_element_set_state(pipeline, GST_STATE_PAUSED);

    _rtp_info->pipeline = pipeline;

    return 0;
}