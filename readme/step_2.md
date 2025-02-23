## 2. 실시간 스트리밍

### 추가 자료
[RPI_CAM](https://github.com/VEDA-Snackticon/RPI-CAM)

[클래스 다이어그램](https://app.diagrams.net/#G1QLGUIHpI_GETmKDeIgp0USpYkadUauDd#%7B%22pageId%22%3A%22LKFbRungoBJc7guEyv_5%22%7D)

### 목표

- v4l2로 지정한 디바이스파일에서 읽어 올 수 있도록함
- v4l2가 아닌 외부에서 연속적으로 넣어주는 영상프레임 경우에도 포멧(ex. YUV422, MJPEG...)에 맞춰서 지원
- mp4파일 재생과 같이 unicast방식이 아닌 multicast방식으로 구현하여 자원을 효율적으로 사용
- 최종적으로는 RPI_CAM의 IP 카메라 프로젝트의 RTSP 서버를 대체(기존 소스가 유니케스트 방식 및 메모리릭 등 문제 많음)

### Multicast with V4L2
- V4L2로 받아온 영상프레임을 RTP 패킷으로 만들어서 Multicast로 전달
#### Requirements
- IGMP Proxyy 혹은 IGMP Snooping을 지원하는 라우터가 필요함
  
> <strong> IGMP </strong>
> 
> Internet Group Management Protocol
> Host와 라우터가 Multicast Group을 구성하는데 사용하는 프로토콜
> 224.0.0.0 - 239.255.255.255 대역의 IP를 사용
> - IGMP Proxy
>   - Multicast 패킷을 라우팅하여 다른 네트워크로 전달
>   - L3 계층
>   - 라우터 하위의 네트워크 노드들에게 Broadcasting을 진행함
> - IGMP Snooping
>   - Multicast 패킷을 필요한 포트로만 전달
>   - L2 계층
>   - 세션에 참여하려는 클라이언트는 IGMP Group에 참여하여 스위치가 해당 패킷을 전송 할 수 있게 해야함

### Multicast with V4L2

- `main()`에서 Multicast Stream에 대한 설정을 끝내고 RTSP서버를 기동해야 함
- 이 때, `RTSPListener`객체의 `path`변수에는 V4L2가 접근하고자 하는 디바이스파일로 설정

#### 객체관계

##### 기존 MP4 파일만을 스트리밍 할 수 있던 방식과의 차이점
- Boost asio가 생성하는 세션 객체까지는 동일
- Session 객체는 RTP 미디어 처리가 아닌 RTSP 패킷을 파싱 및 직렬화 하는 데만 집중하도록 변경
- 별도 Handler 객체를 추가하였으며, Handler 객체에서 RTP 미디어를 처리하도록 함
  
##### Session 객체와 Handler객체를 분화한 이유
- Session 객체와 Handler객체의 책임범위를 단일화
- Session 객체의 생성 시점에 해당 요청이 요구하는 스트림의 정보를 알 수 없기 때문에 Handler객체를 맴버 변수로 소유하면서 Session 객체의 변경없이 유동적으로 변경이 가능하도록 설계

![class_diagram](https://github.com/seongho9/rtsp_server/blob/main/readme/img/multicast_v4l2.png?raw=true)
  
