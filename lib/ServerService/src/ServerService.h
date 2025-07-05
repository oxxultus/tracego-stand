#ifndef WIFI_WEB_SERVICE_H
#define WIFI_WEB_SERVICE_H

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <functional>
#include <WString.h>

/**
 * WebService 클래스
 * - HTTP GET/POST 요청 수신 처리 (서버 역할)
 * - HTTP GET/POST 요청 전송 (클라이언트 역할)
 */
class ServerService {
private:
    int serverPort;                   // HTTP 서버 포트
    WebServer* server = nullptr;     // WebServer 인스턴스를 포인터로 변경

    // 라우팅 핸들러 콜백 함수들 //TODO: 추가해야하는 것
    std::function<String(void)> statusHandler = nullptr;
    std::function<void()> resetConfigHandler = nullptr;

    std::function<String(void)> mainPageHandler = nullptr;
    std::function<String(String)> updateConfigHandler = nullptr;
    std::function<String(void)> advancedPageHandler = nullptr;
    std::function<String(void)> statusViewHandler = nullptr;

    // 선반조작 핸들러
    std::function<String(const String& uid)> startStandHandler = nullptr;
    std::function<String(const String& uid)> upRfidCardHandler = nullptr;
    std::function<String(const String& uid)> downRfidCardHandler = nullptr;

    void setupRoutes();       // 라우팅 등록

public:
    explicit ServerService(int serverPort);    // 생성자
    ~ServerService();                          // 소멸자

    void begin();
    void handle();

    // 핸들러 등록 메서드
    void setStatusHandler(const std::function<String(void)> &handler);
    void setResetConfigHandler(const std::function<void()> &handler);

    void setMainPageHandler(std::function<String(void)> handler);
    void setUpdateConfigHandler(std::function<String(String)> handler);
    void setAdvancedPageHandler(std::function<String(void)> handler);
    void setStatusViewHandler(std::function<String(void)> handler);

    void setStartStandHandler(std::function<String(const String& uid)> handler);
    void setUpRfidCardHandler(std::function<String(const String& uid)> handler);
    void setDownRfidCardHandler(std::function<String(const String& uid)> handler);

    // 핸들러 등록 여부 확인
    [[nodiscard]] bool isStatusHandlerSet() const;
    [[nodiscard]] bool isResetConfigHandlerSet() const;
    [[nodiscard]] bool isMainPageHandlerSet() const;
    [[nodiscard]] bool isUpdateConfigHandlerSet() const;
    [[nodiscard]] bool isAdvancedPageHandlerSet() const;
    [[nodiscard]] bool isStatusViewHandlerSet() const;
    [[nodiscard]] bool isStartStandHandlerSet() const;
    [[nodiscard]] bool isUpRfidCardHandlerSet() const;
    [[nodiscard]] bool isDownRfidCardHandlerSet() const;
};

#endif // WIFI_WEB_SERVICE_H