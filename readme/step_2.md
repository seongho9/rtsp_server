## 2. 실시간 스트리밍

### 목표

- v4l2로 지정한 디바이스파일에서 읽어 올 수 있도록함
- v4l2가 아닌 외부에서 연속적으로 넣어주는 영상프레임 경우에도 포멧(ex. YUV422, MJPEG...)에 맞춰서 지원
- mp4파일 재생과 같이 unicast방식이 아닌 multicast방식으로 구현하여 자원을 효율적으로 사용
- 최종적으로는 RPI_CAM의 IP 카메라 프로젝트의 RTSP 서버를 대체(기존 소스가 유니케스트 방식 및 메모리릭 등 문제 많음)

### Multicast with V4L2
- V4L2로 받아온 영상프레임을 RTP 패킷으로 만들어서 Multicast로 전달
#### Requirements
- IGMP Proxyy 혹은 IGMP Snooping을 지원하는 라우터가 필요함 (쉽게 풀면 공유가기 지원해야함)
  
> <strong> IGMP </strong>
> Internet Group Management Protocol
> Host와 라우터가 Multicast Group을 구성하는데 사용하는 프로토콜
> 224.0.0.0 - 239.255.255.255 대역의 IP를 사용
> - IGMP Proxy
>   - Multicast 패킷을 라우팅하여 다른 네트워크로 전달
>   - L3 계층
>   - 라우터 하위의 네트워크 노드들에게 Broadcasting을 진행함(즉, Wi-Fi와 같은 Broadcasting을 지원하지 않는 경우 사용 할 수 없음
> - IGMP Snooping
>   - Multicast 패킷을 필요한 포트로만 전달
>   - L2 계층
>   - 세션에 참여하려는 클라이언트는 IGMP Group에 참여하여 스위치가 해당 패킷을 전송 할 수 있게 해야함

[RPI_CAM](https://github.com/VEDA-Snackticon/RPI-CAM)

[클래스 다이어그램](https://app.diagrams.net/#G1QLGUIHpI_GETmKDeIgp0USpYkadUauDd#%7B%22pageId%22%3A%22LKFbRungoBJc7guEyv_5%22%7D)
