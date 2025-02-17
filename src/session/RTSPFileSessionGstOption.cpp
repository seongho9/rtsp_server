#include "session/RTSPFileSession.hpp"


namespace asio = boost::asio;
namespace hydra = boost::hydra;

int RTSPFileSessionGst::option_request(std::string& methods)
{
    methods = "DESCRIBE, OPTIONS, SETUP, PLAY, TEARDOWN, PAUSE";

    return 0;
}