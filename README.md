# TraceGo

TraceGo 배송 로봇 시스템에 물건을 전달해주는 선반 입니다.

- 주요 기능
    - 네트워크 연결
    - 내장 서버
    - 보드간 통신

## 하드웨어
- 보드: ESP32-DEVKIT-V1
    - 모듈: DCmoter + moter driver

## 설치

해당 리포지토리를 사용하려면 PlatformIO를 사용해야 합니다.

```
git clone https://github.com/oxxultus/tracego-stand.git
```

- 빌드를 진행한 뒤 업로드를 진행하면됩니다.

### 종속성
- [tracego-wheel](https://github.com/oxxultus/tracego-server.git): `중앙 처리`
