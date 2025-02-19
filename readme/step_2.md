## 2. 실시간 스트리밍

### 목포

- v4l2로 지정한 디바이스파일에서 읽어 올 수 있도록함
- v4l2가 아닌 외부에서 연속적으로 넣어주는 영상프레임 경우에도 포멧(ex. YUV422, MJPEG...)에 맞춰서 지원
- mp4파일 재생과 같이 unicast방식이 아닌 multicast방식으로 구현하여 자원을 효율적으로 사용
- 최종적으로는 RPI_CAM의 IP 카메라 프로젝트의 RTSP 서버를 대체(기존 소스가 유니케스트 방식 및 메모리릭 등 문제 많음)
[RPI_CAM](https://github.com/VEDA-Snackticon/RPI-CAM)
