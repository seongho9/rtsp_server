
#include <spdlog/spdlog.h>

#include "session/RTSPSession.hpp"
#include "handler/RTSPSessionHandler.hpp"
#include "handler/RTSPSessionHandlerLive.hpp"

namespace asio = boost::asio;
namespace hydra = boost::hydra;

static int write_rtsp(RTSPSessionImpl* session, hydra::rtsp::response<hydra::rtsp::string_body>& res);

RTSPSessionImpl::RTSPSessionImpl(
    asio::io_context& ctx, asio::ip::tcp::socket&& socket, std::string uuid, std::unordered_map<std::string, std::string>& path)
    :_context(ctx), _socket(socket), _uuid(uuid), _path(path)
{   
    
    //  소캣 닫힐 때, 데이터 바로 버림
    asio::socket_base::linger linger_opt(true, 0);
    _socket.set_option(linger_opt);


    _url_path.assign("");
    _file_path.assign("");
}

RTSPSessionImpl::~RTSPSessionImpl()
{
    spdlog::debug("RTSP session is destroied");

    if(_handler != nullptr) {
        _handler->destroy();
    }
}

asio::ip::tcp::socket& RTSPSessionImpl::socket()
{
    return _socket;
}

void RTSPSessionImpl::run()
{
    //  소켓이 열려 있는 동안만 반복
    if(!_socket.is_open()){
        spdlog::error("RTSPSession : Socket is already closed");
        destroy();
        return;
    }
    hydra::error_code ec;

    hydra::rtsp::read(_socket, _buffer, _request, ec); 
    if(ec.failed()) {
        //  TCP FIN
        if(ec == hydra::rtsp::error::end_of_stream) {
            spdlog::info("RTSPSession : Close TCP Connection");
            try{
                _socket.shutdown(asio::socket_base::shutdown_receive);
                _socket.close();
            }
            catch(boost::system::system_error& e) {
                spdlog::error("RTSPSession : close tcp error {}", e.what());
            }
        }
        //  Operation aborted
        else if(ec == asio::error::operation_aborted){
            spdlog::error("RTSPSession : Operation Aborted - {} {}", ec.location().function_name(), ec.location().to_string());
            if(_socket.is_open()){
                spdlog::debug("operation aborted so send fin");
                _socket.shutdown(asio::socket_base::shutdown_send);
            }

        }
        //  다른 애러 처리시 추가
        else {
            spdlog::info("RTSPSession : Socket Error {} {}", ec.value(), ec.message());
            _socket.shutdown(asio::socket_base::shutdown_send);
        }
        return;
    }

    if(!read()){
        run();
    }
    else{
        return;
    }
}

int RTSPSessionImpl::read()
{
    hydra::rtsp::response<hydra::rtsp::string_body> res;

    int ret;

    //  세션 초기 생성
    if(_url_path == "" && _file_path == ""){
        _url_path.assign(_request.target());

        //  target의 uri 추출
        size_t idx = _url_path.find("//");
        idx = _url_path.find("/", idx+2);

        _url_path.assign(_url_path.substr(idx, _url_path.length() - idx));

        _file_path.assign(_path[_url_path]);

        if(_file_path.find("/dev") != _file_path.npos) {
            _handler = new RTSPSessionHandlerGstLive(_context);
        }
        else {
            _handler = new RTSPSessionHandlerGstFile(_context);
        }
    }

    res.version(10);

    res.keep_alive(true);
    res.set("CSeq", _request["CSeq"]);
    res.set("Server", "Hydar RTSP Serer");

    //  RTSP OPTION method
    if(_request.method() == hydra::rtsp::verb::options) {

        spdlog::info("RTSPSession : OPTIONS {}", _socket.remote_endpoint().address().to_string());
        
        std::string methods;
        int ret = _handler->option_request(methods);

        //  응답처리
        res.result(hydra::rtsp::status::ok);

        res.set("Public", methods);

        ret = write_rtsp(this, res);
    }
    //  RTSP DESCRIBE method
    else if(_request.method() == hydra::rtsp::verb::describe) {

        spdlog::info("RTSPSession : DESCRIBE {}", _socket.remote_endpoint().address().to_string());

        std::string sdp;
        int ret = _handler->describe_request(_file_path, sdp);

        //  예외처리
        if(!ret) {
            //  응답처리
            res.result(hydra::rtsp::status::ok);

            res.set("Content-Type", "application/sdp");
            res.body() = sdp;
            res.prepare_payload();
            //  hydra 내부에 Content-Length와 Content-Location이 혼용되는 문제가 발생함
            res.set("Content-Length", res["Content-Location"]);
            res.set("Content-Location", _request.target());
        }
        else if(ret == 1) {
            spdlog::error("[RTSPFileSessionGst:read] Failed to get sdp. Reason:pipeline creation");

            res.result(hydra::rtsp::status::internal_server_error);
            res.set("Content-Type", "text/plain");
            res.body() = "SDP Generate Error";
            res.prepare_payload();
            //  hydra 내부에 Content-Length와 Content-Location이 혼용되는 문제가 발생함
            res.set("Content-Length", res["Content-Location"]);
            res.set("Content-Location", _request.target());
        }
        else if(ret == 2) {
            spdlog::warn("[RTSPFileSessionGst:read] Failed to save sdp file.");
            res.result(hydra::rtsp::status::ok);

            res.set("Content-Type", "application/sdp");
            res.body() = sdp;
            res.prepare_payload();
            //  hydra 내부에 Content-Length와 Content-Location이 혼용되는 문제가 발생함
            res.set("Content-Length", res["Content-Location"]);
            res.set("Content-Location", _request.target());
        }
        ret = write_rtsp(this, res);
    }
    //  RTSP SETUP method
    else if(_request.method() == hydra::rtsp::verb::setup) {

        spdlog::info("RTSPSession : SETUP {}", _socket.remote_endpoint().address().to_string());

        //  client 주소
        std::string client_address = _socket.remote_endpoint().address().to_string();
        //  <RTP-PORT>-<RTCP-PORT> 파싱
        size_t client_port_pos = _request["Transport"].find("client_port=") + 12;
        std::string client_ports = _request["Transport"].substr(client_port_pos, _request["Transport"].length() - client_port_pos);

        //  받아올 server가 할당한 포트
        std::string transport;

        int ret = _handler->setup_request(_file_path, client_address, client_ports, transport);
        res.set("Transport", transport);

        //  예외처리
        if(ret) {

        }

        //  응답처리
        res.result(hydra::rtsp::status::ok);
        //  Transport 헤더
        std::string transport_header = _request["Transport"];
        transport_header.append(";").append(transport);

        spdlog::debug("Transport : {}", transport_header);
        //  세션 id는 객체 생성시 생성된 uuid를 사용
        res.set("Session", _uuid);

        ret = write_rtsp(this, res);
    }
    //  RTSP PLAY method
    else if(_request.method() == hydra::rtsp::verb::play) {

        spdlog::info("RTSPSession : PLAY {}", _socket.remote_endpoint().address().to_string());

        std::string range = _request["Range"];
        std::string info;

        int ret = _handler->play_request(range, info);

        //  예외처리
        if(ret) {

        }
        
        //  응답처리
        res.result(hydra::rtsp::status::ok);
        //  RTP-Info 헤더
        std::string rtp_info;
        rtp_info.append("url=").append(_request.target()).append(";").append(info);
        res.set("RTP-Info", rtp_info);
        //  세션 id
        res.set("Session", _uuid);
        //  Range
        res.set("Range", range);

        ret = write_rtsp(this, res);
    }
    //  RTSP PAUSE method
    else if(_request.method() == hydra::rtsp::verb::pause) {

        spdlog::info("RTSPSession : PAUSE {}", _socket.remote_endpoint().address().to_string());

        std::string range = _request["Range"];

        int ret = _handler->pause_request(range);

        //  예외처리
        if(ret) {

        }
        
        //  응답처리
        res.result(hydra::rtsp::status::ok);
        //  세션 id
        res.set("Session", _uuid);
        //  Range
        res.set("Range", range);

        ret = write_rtsp(this, res);
    }
    //  RTSP TEARDOWN method
    else if(_request.method() == hydra::rtsp::verb::teardown) {

        spdlog::info("RTSPSession : TEARDOWN {}", _socket.remote_endpoint().address().to_string());

        std::string range = _request["Range"];

        int ret = _handler->teardown_request();

        //  예외처리
        if(ret) {

        }
        
        //  응답처리
        res.result(hydra::rtsp::status::ok);
        //  세션 id
        res.set("Session", _uuid);
        res.keep_alive(false);

        ret = write_rtsp(this, res);
    }

    return ret;
}

void RTSPSessionImpl::destroy()
{
    delete this;
}

int write_rtsp(RTSPSessionImpl* session, hydra::rtsp::response<hydra::rtsp::string_body>& res)
{

    spdlog::debug("write handler");
    hydra::error_code ec;
    hydra::rtsp::write(session->socket(), res, ec);

    if(ec.failed()){
        spdlog::error("RTSP Response Write Failed : {}", ec.message());
    }

    if(!res.keep_alive()){
        return 1;
    }

    return 0;
}