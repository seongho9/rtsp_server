#include "handler/RTSPSessionHandler.hpp"

int RTSPSessionHandlerGstFile::option_request(std::string& methods)
{
    methods = "DESCRIBE, OPTIONS, SETUP, PLAY, TEARDOWN, PAUSE";
    return 0;
}