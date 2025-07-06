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

// 설정 페이지 조작
void ServerService::setResetConfigHandler(const std::function<void()> &handler) { resetConfigHandler = handler; }
void ServerService::setStatusHandler(const std::function<String(void)> &handler) { statusHandler = handler; }
void ServerService::setMainPageHandler(std::function<String(void)> handler) { mainPageHandler = handler; }
void ServerService::setUpdateConfigHandler(std::function<String(String)> handler) { updateConfigHandler = handler; }
void ServerService::setAdvancedPageHandler(std::function<String(void)> handler) { advancedPageHandler = handler; }
void ServerService::setStatusViewHandler(std::function<String(void)> handler) { statusViewHandler = handler; }
void ServerService::setStartStandHandler(std::function<String(const String& uid)> handler) { startStandHandler = handler; }
void ServerService::setUpRfidCardHandler(std::function<String(const String& uid)> handler) { upRfidCardHandler = handler; }
void ServerService::setDownRfidCardHandler(std::function<String(const String& uid)> handler) { downRfidCardHandler = handler; }

// ========== 라우팅 등록 =====================================================================================
void ServerService::setupRoutes() {
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
            server->send(200, "appljsonication/", result);
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

    if (startStandHandler) {
        server->on("/start-stand", HTTP_GET, [this]() {
            if (!server->hasArg("uid")) {
                server->send(400, "text/plain", "uid 파라미터가 필요합니다");
                return;
            }

            String uid = server->arg("uid");
            
            // 비어있는 uid 방지
            if (uid.length() == 0) {
                server->send(400, "text/plain", "uid는 비어있을 수 없습니다");
                return;
            }

            String result = startStandHandler(uid);  // 외부 핸들러 실행
            

            if (result == "200") {
                server->send(200, "text/plain", "작업 시작됨");
            } else {
                server->send(400, "text/plain", "유효하지 않은 UID");
            }
        });
    }

    if (upRfidCardHandler) {
        server->on("/up-rfid", HTTP_GET, [this]() {
            if (!server->hasArg("uid")) {
                server->send(400, "text/plain", "uid 파라미터가 필요합니다");
                return;
            }

            String uid = server->arg("uid");
            
            // 비어있는 uid 방지
            if (uid.length() == 0) {
                server->send(400, "text/plain", "uid는 비어있을 수 없습니다");
                return;
            }

            String result = upRfidCardHandler(uid);  // 외부 핸들러 실행

            if (result == "200") {
                server->send(200, "text/plain", "작업 시작됨");
            } else {
                server->send(400, "text/plain", "유효하지 않은 UID");
            }
        });
    }

    if (downRfidCardHandler) {
        server->on("/down-rfid", HTTP_GET, [this]() {
            if (!server->hasArg("uid")) {
                server->send(400, "text/plain", "uid 파라미터가 필요합니다");
                return;
            }

            String uid = server->arg("uid");
            
            // 비어있는 uid 방지
            if (uid.length() == 0) {
                server->send(400, "text/plain", "uid는 비어있을 수 없습니다");
                return;
            }

            String result = downRfidCardHandler(uid);  // 외부 핸들러 실행

            if (result == "200") {
                server->send(200, "text/plain", "작업 시작됨");
            } else {
                server->send(400, "text/plain", "유효하지 않은 UID");
            }
        });
    }


}

// ========== 핸들러 등록 여부 확인 ==========================================================================
bool ServerService::isStatusHandlerSet() const { return static_cast<bool>(statusHandler); }
bool ServerService::isResetConfigHandlerSet() const { return static_cast<bool>(resetConfigHandler); }
bool ServerService::isMainPageHandlerSet() const { return static_cast<bool>(mainPageHandler); }
bool ServerService::isUpdateConfigHandlerSet() const { return static_cast<bool>(updateConfigHandler); }
bool ServerService::isAdvancedPageHandlerSet() const { return static_cast<bool>(advancedPageHandler); }
bool ServerService::isStatusViewHandlerSet() const { return static_cast<bool>(statusViewHandler); }
bool ServerService::isStartStandHandlerSet() const { return static_cast<bool>(startStandHandler); }
bool ServerService::isUpRfidCardHandlerSet() const { return static_cast<bool>(upRfidCardHandler); }
bool ServerService::isDownRfidCardHandlerSet() const { return static_cast<bool>(downRfidCardHandler); }