# rtsp_server
boost beast x Gstreamer 기반의 RTSP 서버

## 사용 라이브러리
- boost
  - asio : 비동기 소켓 통신
  - uuids : 세션 id를 지정
- Gstreamer
  - 플러그인
    - filesrc, udpsrc, appsink, qtdemux, queue, h264parse, rtph264pay, rtpbin
  - 사용한 헤더
    - gst/sdp/gstsdpmessage.h
    - gst/app/app.h
    - gst/rtp/rtp.h
    - gst/gst
## 구현 단계
[mp4 파일 재생](https://github.com/seongho9/rtsp_server/blob/main/readme/step_1.md)
