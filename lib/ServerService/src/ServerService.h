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
    std::function<void()> startHandler = nullptr;
    std::function<void()> goHandler = nullptr;
    std::function<void()> stopHandler = nullptr;
    std::function<void()> resetHandler = nullptr;
    std::function<void(const String&)> postHandler = nullptr;

    std::function<String(void)> statusHandler = nullptr;
    std::function<void()> resetConfigHandler = nullptr;

    std::function<String(void)> mainPageHandler = nullptr;
    std::function<String(String)> updateConfigHandler = nullptr;
    std::function<String(void)> advancedPageHandler = nullptr;
    std::function<String(void)> statusViewHandler = nullptr;

    void setupRoutes();       // 라우팅 등록

public:
    explicit ServerService(int serverPort);    // 생성자
    ~ServerService();                          // 소멸자

    void begin();
    void handle();

    // 핸들러 등록 메서드 //TODO: 추가해야하는 것
    void setStartHandler(const std::function<void()> &handler);
    void setGoHandler(const std::function<void()> &handler);
    void setStopHandler(const std::function<void()> &handler);
    void setResetHandler(const std::function<void()> &handler);
    void setPostHandler(const std::function<void(const String&)> &handler);

    void setStatusHandler(const std::function<String(void)> &handler);
    void setResetConfigHandler(const std::function<void()> &handler);

    void setMainPageHandler(std::function<String(void)> handler);
    void setUpdateConfigHandler(std::function<String(String)> handler);
    void setAdvancedPageHandler(std::function<String(void)> handler);
    void setStatusViewHandler(std::function<String(void)> handler);

    // HTTP 요청 전송 메서드
    static String sendGETRequest(const char* host, uint16_t port, const String& pathWithParams);
    static String sendPostRequest(const char* host, uint16_t port, const String& path, const JsonDocument& jsonDoc);

    // 핸들러 등록 여부 확인 //TODO: 추가해야하는 것
    [[nodiscard]] bool isStartHandlerSet() const;
    [[nodiscard]] bool isGoHandlerSet() const;
    [[nodiscard]] bool isStopHandlerSet() const;
    [[nodiscard]] bool isResetHandlerSet() const;

    [[nodiscard]] bool isStatusHandlerSet() const;
    [[nodiscard]] bool isResetConfigHandlerSet() const;
    [[nodiscard]] bool isMainPageHandlerSet() const;
    [[nodiscard]] bool isUpdateConfigHandlerSet() const;
    [[nodiscard]] bool isAdvancedPageHandlerSet() const;
    [[nodiscard]] bool isStatusViewHandlerSet() const;
};

#endif // WIFI_WEB_SERVICE_H