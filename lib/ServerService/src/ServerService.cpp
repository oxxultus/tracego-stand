#include <functional>
#include <WString.h>
#include "ServerService.h"
#include "Config.h"

#include <Preferences.h>
extern Preferences prefs;

// ========== 생성자: 포인터 생성 ==========================================================================
ServerService::ServerService(const int serverPort)
    : serverPort(serverPort)
{
    server = new WebServer(serverPort);
}

// ========== 소멸자: 메모리 해제 ==========================================================================
ServerService::~ServerService() {
    if (server) {
        delete server;
        server = nullptr;
    }
}

// ========== 서버 시작: 라우팅 등록 및 시작 ================================================================
void ServerService::begin() {
    setupRoutes();
    server->begin();
    Serial.println("[ServerService][1/2] TraceGo-Stand의 내장 HTTP 서버가 시작되었습니다.");
}

void ServerService::handle() {
    server->handleClient();
}

// ========== 핸들러 등록 ====================================================================================
void ServerService::setStartHandler(const std::function<void()> &handler) { startHandler = handler; }            // 카트 조작
void ServerService::setGoHandler(const std::function<void()> &handler)    { goHandler = handler; }               // 카트 조작
void ServerService::setStopHandler(const std::function<void()> &handler)  { stopHandler = handler; }             // 카트 조작
void ServerService::setResetHandler(const std::function<void()> &handler) { resetHandler = handler; }            // 카트 조작
void ServerService::setPostHandler(const std::function<void(const String&)> &handler) { postHandler = handler; } // 카트 조작

// 설정 페이지 조작
void ServerService::setResetConfigHandler(const std::function<void()> &handler) { resetConfigHandler = handler; }
void ServerService::setStatusHandler(const std::function<String(void)> &handler) { statusHandler = handler; }
void ServerService::setMainPageHandler(std::function<String(void)> handler) { mainPageHandler = handler; }
void ServerService::setUpdateConfigHandler(std::function<String(String)> handler) { updateConfigHandler = handler; }
void ServerService::setAdvancedPageHandler(std::function<String(void)> handler) { advancedPageHandler = handler; }
void ServerService::setStatusViewHandler(std::function<String(void)> handler) { statusViewHandler = handler; }
// ========== GET/POST 요청 전송 =============================================================================
String ServerService::sendGETRequest(const char* host, const uint16_t port, const String& pathWithParams) {
    String response = "";
    WiFiClient client;
    if (client.connect(host, port)) {
        client.print(String("GET ") + pathWithParams + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Connection: close\r\n\r\n");

        unsigned long timeout = millis() + 3000;
        while (client.connected() && millis() < timeout) {
            while (client.available()) {
                response += (char)client.read();
            }
        }
        client.stop();
    }
    return response;
}

String ServerService::sendPostRequest(const char* host, uint16_t port, const String& path, const JsonDocument& jsonDoc) {
    String response = "";
    WiFiClient client;
    if (client.connect(host, port)) {
        String jsonString;
        serializeJson(jsonDoc, jsonString);

        client.print(String("POST ") + path + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Content-Type: application/json\r\n" +
                     "Content-Length: " + jsonString.length() + "\r\n\r\n" +
                     jsonString);

        while (client.connected() || client.available()) {
            if (client.available()) {
                response += client.readStringUntil('\n');
            }
        }
        client.stop();
    }
    return response;
}

// ========== 라우팅 등록 =====================================================================================
void ServerService::setupRoutes() {
    if (startHandler) {
        server->on("/start", HTTP_GET, [this]() {
            startHandler();
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "application/json", "{\"message\":\"Handled GET /start\"}");
        });
    }

    if (goHandler) {
        server->on("/go", HTTP_GET, [this]() {
            goHandler();
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "application/json", "{\"message\":\"Handled GET /go\"}");
        });
    }

    if (stopHandler) {
        server->on("/stop", HTTP_GET, [this]() {
            stopHandler();
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "application/json", "{\"message\":\"Handled GET /stop\"}");
        });
    }

    if (resetHandler) {
        server->on("/reset", HTTP_GET, [this]() {
            resetHandler();
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "application/json", "{\"message\":\"Handled GET /reset\"}");
        });
    }
    
    if (postHandler) {
        server->on("/post", HTTP_POST, [this]() {
            String body = server->arg("plain");
            postHandler(body);
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "application/json", "{\"message\":\"Handled POST /post\"}");
        });
    }

    if (statusHandler) {
        server->on("/status", HTTP_GET, [this]() {
            String json = statusHandler();
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "application/json", json);
        });
    }

    if (resetConfigHandler) {
        server->on("/reset-config", HTTP_GET, [this]() {
            resetConfigHandler();
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "application/json", "{\"message\":\"설정 초기화됨. 재시작합니다.\"}");
            delay(1000);
            ESP.restart();
        });
    }


    if (mainPageHandler) {
        server->on("/", HTTP_GET, [this]() {
            String html = mainPageHandler();
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "text/html; charset=utf-8", html);
        });
    }

    if (advancedPageHandler) {
        server->on("/advanced", HTTP_GET, [this]() {
            String html = advancedPageHandler();
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "text/html; charset=utf-8", html);
        });
    }

    if (updateConfigHandler) {
        server->on("/update-config", HTTP_POST, [this]() {
            String body = server->arg("plain");
            String result = updateConfigHandler(body);  // ← 여기서 전달
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "application/json", result);
            delay(3000);
            ESP.restart();
        });
    }

    if (statusViewHandler) {
        server->on("/status-view", HTTP_GET, [this]() {
            String html = statusViewHandler();
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "text/html; charset=utf-8", html);
        });
    }
}

// ========== 핸들러 등록 여부 확인 ==========================================================================
bool ServerService::isStartHandlerSet() const { return static_cast<bool>(startHandler); }
bool ServerService::isGoHandlerSet() const { return static_cast<bool>(goHandler); }
bool ServerService::isStopHandlerSet() const { return static_cast<bool>(stopHandler); }
bool ServerService::isResetHandlerSet() const { return static_cast<bool>(resetHandler); }

bool ServerService::isStatusHandlerSet() const { return static_cast<bool>(statusHandler); }
bool ServerService::isResetConfigHandlerSet() const { return static_cast<bool>(resetConfigHandler); }
bool ServerService::isMainPageHandlerSet() const { return static_cast<bool>(mainPageHandler); }
bool ServerService::isUpdateConfigHandlerSet() const { return static_cast<bool>(updateConfigHandler); }
bool ServerService::isAdvancedPageHandlerSet() const { return static_cast<bool>(advancedPageHandler); }
bool ServerService::isStatusViewHandlerSet() const { return static_cast<bool>(statusViewHandler); }