#include "handler/RTSPSessionHandler.hpp"

#include "spdlog/spdlog.h"

RTSPSessionHandlerGstFile::RTSPSessionHandlerGstFile(boost::asio::io_context& context)
    :   _context(context)
{   }


RTSPSessionHandlerGstFile::~RTSPSessionHandlerGstFile()
{
    if(_rtp_info != nullptr) {
        spdlog::debug("delete gst handler : rtp info");
        delete _rtp_info;
    }
    if(_codec_info != nullptr) {
        spdlog::debug("delete gst handler : codec info");
        delete _codec_info;
    }
}

int RTSPSessionHandlerGstFile::destroy()
{
    delete this;

    return 0;
}