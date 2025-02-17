# RTSP Server

기존 구현되어 있는 RTSP 서버를 사용하지 않고, 더 세밀한 제어가 가능하도록 직접 RTSP 구현을 진행

- 사용한 라이브러리

  - boost
    - asio : 비동기 소켓 통신
    - uuids : 세션 id를 지정
  - Gstreamer
    - 미디어 스트림 생성(H.264)
    - SDP 생성

- 개작한 라이브러리

  - boost :: beast

    RTSP와 HTTP 프로토콜 간 유사성에 기반하여 HTTP 프로토콜 라이브러리인 boost::beast의 메소드, 헤더, 응답 상태 값을 변경하여 RTSP 패킷을 파싱하도록 변경

    현 소스 코드에서는 해당 개작 라이브러리를 hydra로 칭하고 있으며, beast 네임스페이스는 hydra로 http 네임스페이스는 rtsp로 변경하였음

## RTSP 구현 사항

우선은 실시간 스트림이 아닌 기 존재하는 mp4파일을 읽어 클라이언트에 전송하는 것을 1차 목표로 진행(2025.02.17)

### 1. MP4 스트리밍

- 구현 메소드

  OPTION, DESCRIBE, SETUP, PLAY, PAUSE, TEAERDOWN

- DESCRIBE

  Gstreamer의 파이프라인은 다음과 같이 구성

  ```
  filesrc -> qtdemux -> (video) queue -> h264parse -> rtph264pay -> appsink
  				  -> (audio) ...
  ```

  이 때, 구성된 RTP 패킷을 `appsink` Element로 받아와 SDP 생성에 필요한 값들을 가져옴

  이 때 가져온 메타데이터는 `encoding-name`, `sprop-parameter-sets`, `clock-rate`, `ssrc`, `a-framerate`, `profile-level-id`, `packetization-mode` 임

  이를 `appsink` element의 `new-sample` 시그널에 아래 함수를 콜백으로 등록하여 위의 값들을 가져옴

  ```cpp
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
  ```

- SETUP

  이 부분은 클라이언트가 요청한 자원에 대해 스트리밍을 진행하기 위해 미디어를 준비하는 메소드로 세션 ID도 이 단계에서 서버가 생성

  이 서버의 경우 GET_PARAMETER, SET_PARAMETER 메소드를 구현하지 않았으므로, ping 혹은 스트리밍 정보 를 RTCP를 통해 주고 받아야 한다.

  파이프라인은 다음과 같이 구성

  ```
  filesrc -> qtdemux -> (video) queue -> h264parse -> rtph264pay -> 
  rtpbin:send_rtp_sink_0 -> rtpbin:send_rtp_src_0 -> appsink
  rtpbin:send_rtcp_sink_0 -> appsink
  udpsrc -> rtpbin:recv_rtcp_src_0
  ```

  이 때, `udpsink`가 아닌 `appsink`를 통해 RTP 패킷을 전송하는 이유는 

  첫번째, 클라이언트에게 서버의 RTP, RTCP 포트 번호를 전송해야 하는데, `udpsink`의 경우 동적으로 포트를 할당 한 후 재생하기 전까지 이를 알 수 없기 때문에 boost::asio를 이용 비동기 udp 소캣을 구현하였으며,

  두번째, 클라이언트에게 SETUP에 대한 응답으로 제공해야 되는 정보 중 첫 패킷의 rtp-time과 seqeunce번호를 제공해야 하는데 이는 RTP 패킷의 페이로드를 봐야하기 때문에 이렇게 구현하였음

> <strong>DESCRIBE vs. SETUP</strong>
>
> - DESCRIBE
>
>   클라이언트가 요청한 자원에 대한 정보를 제공
>
>   이 때, 정보는 미디어의 코덱정보와 같은 재생에 필요한 스트림의 정보임
>
>   ```
>   Real Time Streaming Protocol
>       Request: DESCRIBE rtsp://10.0.0.3:8550/test RTSP/1.0\r\n
>       CSeq: 3\r\n
>       User-Agent: LibVLC/3.0.21 (LIVE555 Streaming Media v2016.11.28)\r\n
>       Accept: application/sdp\r\n
>       \r\n
>   
>   Real Time Streaming Protocol
>       Response: RTSP/1.0 200 OK\r\n
>       CSeq: 3\r\n
>       Server: Hydra RTSP Serer\r\n
>       Content-type: application/sdp
>       Content-length: 280
>       Content-Location: rtsp://10.0.0.3:8550/test\r\n
>       \r\n
>       Session Description Protocol
>           Owner/Creator, Session Id (o): - 1739763831 0 IN IP4 10.0.0.3
>           Session Name (s): Hydra RTSP Session
>           Time Description, active time (t): 0 0
>           Media Description, name and address (m): video 0 RTP/AVP 96
>           Media Attribute (a): fmtp:96 packetization-mode=1;sprop-parameter-sets=Z2QAKKzZgHgCJ+WagICAoAAAAwAgAB1MAeMGM0A=,aOl7LIs=;profile-level-id=29.970029970029969
>           Media Attribute (a): rtpmap:96 H264/90000
>           Media Attribute (a): framerate:29.970029970029969
>   
>   ```
>
> - SETUP
>
>   클라이언트가 재생을 하기위해 서버에서 해당 자원을 재생 할 수 있도록 준비함과 동시에 재생을 위한 네트워크에 대한 정보를 제공
>
>   이 때, 정보는 클라이언트 RTP, RTCP 포트, 서버 RTP, RTCP포트, 서버에서 생성한 세션 ID
>
>   ```
>   Real Time Streaming Protocol
>       Request: SETUP rtsp://10.0.0.3:8550/test/ RTSP/1.0\r\n
>       CSeq: 4\r\n
>       User-Agent: LibVLC/3.0.21 (LIVE555 Streaming Media v2016.11.28)\r\n
>       Transport: RTP/AVP;unicast;client_port=60696-60697
>       \r\n
>   
>   Real Time Streaming Protocol
>       Response: RTSP/1.0 200 OK\r\n
>       Connection: keep-alive\r\n
>       CSeq: 4\r\n
>       Server: Hydra RTSP Serer\r\n
>       Transport: RTP/AVP;unicast;client_port=60696-60697;server_port=46007-46008
>       Session: b1076b6d-37c9-49c3-8238-48958d840134
>       \r\n
>   ```

- PLAY

  SETUP에서 준비한 미디어를 재생하는 메소드

  이 때 응답으로 클라이언트가 재생을 위해 필요한 첫 RTP 패킷의 시퀀스 번호(`seq=`), rtptime(`rtptime=`)을 제공해야 한다.

  이 서버에서는 위에서 준비한 Gstreamer 파이프라인을 `GST_STATE_PLAYING`으로 바꾸면서 진행

- PAUSE

  재생을 일시정지하는 메소드

  위 PLAY에서 Gstreamer의 파이프라인을 `GST_STATE_PAUSED`로 변경하는 것 말고는 동일

- TEARDOWN

  해당 RTSP 세션을 종료하는 메소드

  Gstreamer의 파이프라인을 멈추고(`GST_STATE_NULL`) 관련 자원(소켓, 객체)를 할당 해제

- 객체 관계

  ![class](C:\Users\SeonghoJang\Desktop\rtsp_server\rtsp_class.png)
