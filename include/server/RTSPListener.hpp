#ifndef _RTSP_LISTNER_HPP
#define _RTSP_LISTNER_HPP

#include <unordered_map>
#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

#include <boost/config.hpp>


#include "session/RTSPFileSession.hpp"

class RTSPListener
{
public:
    virtual void do_accept()=0;
    virtual void handle_session(boost::asio::ip::tcp::socket&& socket)=0;
};

class RTSPListenerImpl 
    : public RTSPListener
{
private:
    
    //  url_path - file_path
    std::unordered_map<std::string, std::string> _path;
    
    //  async i/o context
    boost::asio::io_context& _context;
    //  rtsp request acceptor
    boost::asio::ip::tcp::acceptor _acceptor;
    
    //  uuid generator
    boost::uuids::random_generator _generator;


public:
    RTSPListenerImpl() = delete;
    RTSPListenerImpl(boost::asio::io_context& ctx, boost::asio::ip::tcp::endpoint endpoint, std::unordered_map<std::string, std::string> path);

    void do_accept() override;
    void handle_session(boost::asio::ip::tcp::socket&& socket) override;
};

#endif