#include <Arduino.h>
#include <Preferences.h>
#include <HTTPClient.h>

#include "Config.h"
#include "ConfigWebServer.h"
#include "ServerService.h"
#include "WiFiConnector.h"

// 함수 선언부 ===========================================================================================================
void modulsSetting();                                               // [SETUP-1] 모듈을 초기 설정 하는 함수입니다.
void setServerHandler();                                            // [SETUP-2] 핸들러 등록을 진행하는 함수입니다.
void handleStartStandRequest(const String& uid);                    // [UTILITY-1] 작업 수행을 시작하는 함수 입니다.
void sendCheckWorkingRequest(const String& uid);                    // [UTILITY-2] 작업 아이템을 확인하는 함수입니다.
void sendEndWorkingRequest(const String& uid);                      // [UTILITY-3] 작업 아이템을 완료하는 함수입니다.
void startStandMoter(const String& uid, const int& count);          // [MOTER-1] 선반 조작을 시작하는 함수입니다.
void upRfidCard(const String& uid);                                 // [MOTER-2] RFID 카드 올리기 함수입니다.
void downRfidCard(const String& uid);                               // [MOTER-3] RFID 카드 내리기 함수입니다;

// 객체 생성 =============================================================================================================
WiFiConnector wifi;                             // WiFiConnect 객체 생성
ServerService* serverService = nullptr;         // WebService 객체 생성
ConfigWebServer* configWebServer = nullptr;     // ConfigWebServer 객체 생성

// 프로그램 설정 및 시작 ====================================================================================================

void setup() {
    config.load(); // EEPROM 또는 Preferences에서 구성 불러오기

    Serial.begin(config.serialBaudrate);  // 시리얼 초기화 (최우선)

    // 객체 동적 생성
    wifi = WiFiConnector(config.ssid.c_str(), config.password.c_str());
    if (!wifi.connect(5000)) {  // 5초 내 미연결 시 설정 모드 전환
        Serial.println("[TraceGo_Stand_Setting][WiFi] 연결 실패. 설정 모드로 진입합니다.");

        WiFi.mode(WIFI_AP);
        WiFi.softAP("TraceGo_Stand_Setting", "12345678");  // SoftAP 시작
        Serial.println("[TraceGo_Stand_Setting][WiFi] SoftAP 모드 활성화: SSID = TraceGo_Stand_Setting 접속 IP: " + WiFi.softAPIP().toString());
        configWebServer = new ConfigWebServer(config);
        configWebServer->begin();
        return; // loop에서 configWeb 핸들러로 진입하게 됨
    }
    serverService = new ServerService(config.innerPort);

    modulsSetting();           // 모듈 초기 설정
    setServerHandler();        // 서버 핸들러 등록
    serverService->begin();    // 서버 시작

    Serial.println("[TraceGo_Stand_Setting][MAIN] 선반 코어 준비 완료");
}

void loop() {
    if (configWebServer) {
        configWebServer->handleClient();            // 설정 모드일 경우 처리
        return;
    }

    if (serverService) {serverService->handle();}   // 내장 서버 구동
    delay(1);                                       // WDT 리셋 방지
}

// SETUP FUNCTION =====================================================================================================

// [SETUP-1] 모듈을 초기 설정 하는 함수입니다.
void modulsSetting() {
    Serial.begin(config.serialBaudrate);            // 시리얼 설정

    wifi.connect();                                 // wifi 연결
}

// [SETUP-2] 핸들러 등록을 진행하는 함수입니다.
void setServerHandler() {
    
    // [메인 페이지 핸들러] 기본 설정 페이지를 반환하는 핸들러입니다.
    serverService->setMainPageHandler([]() -> String {
        return R"rawliteral(
            <!DOCTYPE html>
            <html lang="ko">
            <head>
                <meta charset="utf-8">
                <title>TraceGo-Stand 설정 페이지</title>
                <style>
                    * { box-sizing: border-box; }
                    body {
                        font-family: 'Segoe UI', sans-serif;
                        background-color: #f4f7f8;
                        margin: 0;
                        padding: 0;
                        display: flex;
                        justify-content: center;
                        align-items: center;
                        height: 100vh;
                    }
                    .container {
                        background-color: #fff;
                        padding: 40px;
                        border-radius: 12px;
                        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
                        text-align: center;
                        width: 100%;
                        max-width: 400px;
                    }
                    h2 {
                        margin-bottom: 30px;
                        color: #00c4c4;
                    }
                    a {
                        display: block;
                        margin: 12px 0;
                        padding: 12px;
                        background-color: #00c4c4;
                        color: #fff;
                        text-decoration: none;
                        border-radius: 8px;
                        font-size: 16px;
                        transition: background-color 0.3s ease;
                    }
                    a:hover {
                        background-color: #00a0a0;
                    }
                </style>
            </head>
            <body>
                <div class="container">
                    <h2>TraceGo-Stand 설정 페이지</h2>
                    <a href="/advanced">고급 설정</a>
                    <a href="/status-view">상태 확인</a>
                    <a href="/reset-config">설정 초기화</a>
                </div>
            </body>
            </html>
        )rawliteral";
    });

    // [고급 설정 핸들러] 고급 설정 페이지를 반환하는 핸들러입니다.
    serverService->setAdvancedPageHandler([]() -> String {
        String html = R"rawliteral(
            <!DOCTYPE html>
            <html lang="ko">
            <head>
                <meta charset="utf-8">
                <title>TraceGo-Stand 고급 설정</title>
                <style>
                    * { box-sizing: border-box; }
                    body {
                        font-family: 'Segoe UI', sans-serif;
                        background-color: #f4f7f8;
                        margin: 0;
                        padding: 0;
                        display: flex;
                        justify-content: center;
                        align-items: flex-start;
                        min-height: 100vh;
                    }
                    .container {
                        width: 100%;
                        max-width: 600px;
                        background: #fff;
                        padding: 30px;
                        margin: 40px auto;
                        border-radius: 12px;
                        box-shadow: 0 4px 10px rgba(0,0,0,0.1);
                    }
                    h2 {
                        text-align: center;
                        color: #00c4c4;
                        margin-bottom: 20px;
                    }
                    fieldset {
                        border: none;
                        margin-bottom: 20px;
                        padding: 0;
                    }
                    legend {
                        font-weight: bold;
                        color: #00a0a0;
                        margin-bottom: 10px;
                    }
                    label {
                        display: block;
                        margin-bottom: 6px;
                        font-weight: 500;
                    }
                    input[type=text],
                    input[type=password],
                    input[type=number] {
                        width: 100%;
                        padding: 10px;
                        margin-bottom: 14px;
                        border: 1px solid #ccc;
                        border-radius: 6px;
                        font-size: 14px;
                    }
                    input[type=checkbox] {
                        transform: scale(1.2);
                        margin-left: 4px;
                    }
                    button {
                        width: 100%;
                        padding: 14px;
                        background-color: #00c4c4;
                        color: #fff;
                        border: none;
                        border-radius: 6px;
                        font-size: 16px;
                        cursor: pointer;
                        transition: background-color 0.3s;
                    }
                    button:hover {
                        background-color: #00a0a0;
                    }
                </style>
                <script>
                function saveConfig() {
                    const config = {
                        server_ip: document.getElementById("server_ip").value,
                        server_port: parseInt(document.getElementById("server_port").value),
                        inner_port: parseInt(document.getElementById("inner_port").value),
                        baudrate: parseInt(document.getElementById("baudrate").value),
                        baudrate2: parseInt(document.getElementById("baudrate2").value),
                        checkWorkingList: document.getElementById("cwl").value,
                        endWorkingList: document.getElementById("ewl").value,
                    };

                    fetch("/update-config", {
                        method: "POST",
                        headers: { "Content-Type": "application/json" },
                        body: JSON.stringify(config)
                    })
                    .then(res => res.json())
                    .then(data => alert(data.message));
                }
                </script>
            </head>
            <body>
                <div class="container">
                    <h2>TraceGo-Stand 고급 설정</h2>

                    <fieldset>
                        <legend>서버 설정</legend>
                        <label for="server_ip">Server IP</label>
                        <input id="server_ip" value="%SERVER_IP%" type="text">

                        <label for="server_port">Server Port</label>
                        <input id="server_port" value="%SERVER_PORT%" type="number">

                        <label for="inner_port">Inner Port</label>
                        <input id="inner_port" value="%INNER_PORT%" type="number">
                    </fieldset>

                    <fieldset>
                        <legend>하드웨어 설정</legend>
                        <label for="baudrate">Baudrate</label>
                        <input id="baudrate" value="%BAUDRATE%" type="number">

                        <label for="baudrate2">Baudrate2</label>
                        <input id="baudrate2" value="%BAUDRATE2%" type="number">
                    </fieldset>

                    <fieldset>
                        <legend>엔드포인트 설정</legend>
                        <label for="cwl">CheckWorkingList</label>
                        <input id="cwl" value="%CWL%" type="text">

                        <label for="ewl">EndWorkingList</label>
                        <input id="ewl" value="%EWL%" type="text">
                    </fieldset>

                    <fieldset>
                        <legend>상품 UID 설정</legend>
                        <label for="fpu">FirstProductUID</label>
                        <input id="fpu" value="%FPU%" type="text">

                        <label for="spu">SecondProductUID</label>
                        <input id="spu" value="%SPU%" type="text">
                    </fieldset>

                    <button onclick="saveConfig()">설정 저장</button>
                </div>
            </body>
            </html>
        )rawliteral";

        // 치환
        html.replace("%SERVER_IP%", config.serverIP);
        html.replace("%SERVER_PORT%", String(config.serverPort));
        html.replace("%INNER_PORT%", String(config.innerPort));
        html.replace("%BAUDRATE%", String(config.serialBaudrate));
        html.replace("%BAUDRATE2%", String(config.serial2Baudrate));

        html.replace("%CWL%", config.checkWorkingList);
        html.replace("%EWL%", config.endWorkingList);

        html.replace("%FPU%", config.firstProductUid);
        html.replace("%SPU%", config.secondProductUid);
        return html;
    });

    // [고급 설정 핸들러] 고급 설정 변경사항을 저장하는 핸들러입니다.
    serverService->setUpdateConfigHandler([](String body) -> String {
        JsonDocument doc;
        doc.set(JsonObject());
        DeserializationError err = deserializeJson(doc, body);
        if (err) return "{\"message\":\"JSON 파싱 실패\"}";

        extern Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("server_ip", doc["server_ip"] | "");
        prefs.putInt("server_port", doc["server_port"] | 8080);
        prefs.putInt("inner_port", doc["inner_port"] | 8082);
        prefs.putInt("baudrate", doc["baudrate"] | 115200);
        prefs.putInt("baudrate2", doc["baudrate2"] | 9600);
        prefs.putString("cwl", doc["checkWorkingList"] | "/check/working-list?uid=");
        prefs.putString("ewl",  doc["endWorkingList"]   | "/end/working-list?uid=");
        prefs.putString("fpu", doc["firstProductUid"] | "c334db26");
        prefs.putString("spu",  doc["secondProductUid"]   | "9a812ae7");
        prefs.end();

        return "{\"message\":\"설정이 저장되었습니다. 3초 후 재시작됩니다.\"}";
    });

    // [상태 핸들러] 현재 시스템 상태를 JSON 형태로 반환하는 핸들러입니다.
    serverService->setStatusHandler([]() -> String {
        JsonDocument doc;  // 권장된 JsonDocument 타입 사용
        doc.set(JsonObject());  // 명시적 초기화 (v7에서는 안전하게 사용하기 위해 권장됨)

        doc["ssid"]                 = config.ssid;
        doc["password"]             = config.password;
        doc["server_ip"]            = config.serverIP;
        doc["server_port"]          = config.serverPort;
        doc["inner_port"]           = config.innerPort;
        doc["localIP"]              = config.localIP;
        doc["baudrate"]             = config.serialBaudrate;
        doc["baudrate2"]            = config.serial2Baudrate;
        doc["checkWorkingList"]     = config.checkWorkingList;
        doc["endWorkingList"]       = config.endWorkingList;
        doc["firstProductUid"]      = config.firstProductUid;
        doc["secondProductUid"]     = config.secondProductUid;
        
        String output;
        serializeJson(doc, output);
        return output;
    });

    // [상태 뷰 핸들러] 시스템 상태를 HTML로 표시하는 핸들러입니다.
    serverService->setStatusViewHandler([]() -> String {
        return  R"rawliteral(
                <!DOCTYPE html>
                <html lang="ko">
                <head>
                    <meta charset="utf-8">
                    <title>TraceGo-Stand 시스템 상태</title>
                    <style>
                        body { font-family: 'Segoe UI', sans-serif; margin: 20px; background: #f4f7f8; }
                        pre {
                            background: #fff;
                            padding: 20px;
                            border-radius: 10px;
                            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
                            overflow-x: auto;
                            white-space: pre-wrap;
                        }
                        h2 { color: #00c4c4; }
                    </style>
                </head>
                <body>
                    <h2>TraceGo-Stand 시스템 상태</h2>
                    <pre id="status">불러오는 중...</pre>

                    <script>
                        fetch("/status")
                            .then(response => response.json())
                            .then(data => {
                                document.getElementById("status").textContent = JSON.stringify(data, null, 2);
                            })
                            .catch(error => {
                                document.getElementById("status").textContent = "불러오기 실패: " + error;
                            });
                    </script>
                </body>
                </html>
            )rawliteral";
    });

    // [설정 초기화 핸들러] 모든 설정을 초기화하는 핸들러입니다.
    serverService->setResetConfigHandler([]() {
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.clear();  // 모든 설정 삭제
        prefs.end();
    });
    
    // TODO: 마이크로 서보 모터 작동로직 추가
    // RFID 카드를 들어올리는 로직이 필요하다 외부에 접근 가능하게 해야한다 예) 카트-> 선반으로 명령전달 시 로직이 실행되어야함

    serverService->setStartStandHandler([](const String& uid) -> String {
        Serial.println("[외부 핸들러] /start-stand 요청: UID = " + uid);

        // 상품 UID가 유효한지 확인
        if (uid == config.firstProductUid || uid == config.secondProductUid) {
            Serial.println("[외부 핸들러] /start-stand 요청: 유효한 UID");

            // 작업 아이템 확인 요청
            handleStartStandRequest(uid);

            return "200";
        } else {
            Serial.println("[외부 핸들러] /start-stand 요청: 유효하지 않은 UID");
            return "400";
        }
    });

    serverService->setUpRfidCardHandler([](const String& uid) -> String {
        Serial.println("[외부 핸들러] /up-rfid 요청: UID = " + uid);

        // 상품 UID가 유효한지 확인
        if (uid == config.firstProductUid || uid == config.secondProductUid) {
            Serial.println("[외부 핸들러] /up-rfid 요청: 유효한 UID");

            // RFID 카드 들어올리기
            upRfidCard(uid);

            return "200";
        } else {
            Serial.println("[외부 핸들러] /up-rfid 요청: 유효하지 않은 UID");
            return "400";
        }
    });

    serverService->setDownRfidCardHandler([](const String& uid) -> String {
        Serial.println("[외부 핸들러] /down-rfid 요청: UID = " + uid);

        // 상품 UID가 유효한지 확인
        if (uid == config.firstProductUid || uid == config.secondProductUid) {
            Serial.println("[외부 핸들러] /down-rfid 요청: 유효한 UID");

            // RFID 카드 내리기
            downRfidCard(uid);

            return "200";
        } else {
            Serial.println("[외부 핸들러] /down-rfid 요청: 유효하지 않은 UID");
            return "400";
        }
    });

    Serial.println("[setServerHandler][1/2] 내장 서버 API 실행 함수 등록 확인 절차 시작");
        auto printHandlerStatus = [](const char* name, bool status) {
            Serial.print("[");
            Serial.print(status ? "\u2714" : "\u2718"); // ✔ 또는 ✘
            Serial.print("] ");
            Serial.print(name);
            Serial.println(" 핸들러 등록 " + String(status ? "완료" : "실패"));
        };
        printHandlerStatus("/", serverService->isMainPageHandlerSet());
        printHandlerStatus("/advanced",  serverService->isAdvancedPageHandlerSet());
        printHandlerStatus("/update-config", serverService->isUpdateConfigHandlerSet());
        printHandlerStatus("/status",    serverService->isStatusHandlerSet());
        printHandlerStatus("/status-view", serverService->isStatusViewHandlerSet());
        printHandlerStatus("/reset-config",  serverService->isResetConfigHandlerSet());
        printHandlerStatus("/start-stand", serverService->isStartStandHandlerSet());
        printHandlerStatus("/up-rfid", serverService->isUpRfidCardHandlerSet());
        printHandlerStatus("/down-rfid", serverService->isDownRfidCardHandlerSet());
    Serial.println("[setServerHandler][2/2] 내장 서버 API 실행 함수 등록 절차 완료\n");
}

// UTILITY FUNCTION =======================================================================================================

// [UTILITY-1] 작업 수행을 시작하는 함수 입니다.
void handleStartStandRequest(const String& uid) {
    sendCheckWorkingRequest(uid);
}

// [UTILITY-2] 작업 아이템을 확인하는 함수입니다.
void sendCheckWorkingRequest(const String& uid) {
    HTTPClient http;

    // 1. check 요청 URL 생성
    String checkUrl = "http://" + String(config.serverIP) + ":" + String(config.serverPort) + String(config.checkWorkingList) + uid;

    int code = -1;
    String body;

    // 최대 3회 재시도
    const int maxAttempts = 3;
    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        Serial.println("[" + String(attempt) + "차 시도] 작업 요청 중: " + checkUrl);

        http.begin(checkUrl);
        code = http.GET(); // check 응답 코드
        Serial.println("HTTP 상태 코드: " + String(code));

        if (code > 0) {
            // 정상 응답이면 본문 수신 및 처리
            if (code == 200) {
                body = http.getString();
                Serial.println("작업 있음 → 응답 내용: " + body);

                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, body);

                if (error) {
                    Serial.print("JSON 파싱 실패: ");
                    Serial.println(error.c_str());
                } else {
                    if (doc["count"].is<int>()) {
                        int count = doc["count"];
                        Serial.println("작업 수량 (count): " + String(count));
                        startStandMoter(uid, count); // 선반 조작 시작
                    } else {
                        Serial.println("응답에 'count' 필드가 없습니다.");
                    }
                }
            } else if (code == 204) {
                Serial.println("서버에 작업 리스트가 없음 (204)");
            } else if (code == 404) {
                Serial.println("UID에 해당하는 작업 없음 (404)");
            } else {
                Serial.println("예상 외 응답 코드: " + String(code));
            }

            http.end(); // 요청 종료
            return; // 정상 응답 받았으므로 함수 종료
        } else {
            Serial.print("요청 실패 (오류): ");
            Serial.println(http.errorToString(code));
            http.end();
            delay(500); // 다음 시도 전 잠깐 대기
        }
    }

    // 재시도 후에도 실패한 경우
    Serial.println("⚠️ 요청 실패 - 최대 재시도 횟수 초과 (" + String(maxAttempts) + "회)");
}

// [UTILITY-3] 작업 아이템을 완료하는 함수입니다.
void sendEndWorkingRequest(const String& uid) {
    HTTPClient endHttp;
    String endUrl = "http://" + String(config.serverIP) + ":" + String(config.serverPort) + String(config.endWorkingList) + uid;

    int endCode = -1;
    String endBody;

    const int maxAttempts = 3;
    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        Serial.println("[" + String(attempt) + "차 시도] 작업 완료 요청 중: " + endUrl);

        endHttp.begin(endUrl);
        endCode = endHttp.GET();

        if (endCode > 0) {
            if (endCode == 200) {
                endBody = endHttp.getString();
                Serial.println("작업 완료 요청 성공 → 응답 내용: " + endBody);
            } else {
                Serial.println("작업 완료 요청 실패 → 응답 코드: " + String(endCode));
            }

            endHttp.end();
            return; // 성공했거나 실패했더라도 응답 받았으면 종료
        } else {
            Serial.print("요청 실패 (오류): ");
            Serial.println(endHttp.errorToString(endCode));
            endHttp.end();
            delay(500); // 재시도 전 대기
        }
    }

    // 재시도 끝난 후에도 실패한 경우
    Serial.println("⚠️ 작업 완료 요청 실패 - 최대 재시도 횟수 초과 (" + String(maxAttempts) + "회)");
}

// MOTER FUNCTION =========================================================================================================

// [MOTER-1] 선반 조작을 시작하는 함수입니다.
void startStandMoter(const String& uid, const int& count) {
    if (uid == config.firstProductUid) {
        Serial.println("[MOTER] 선반 조작 시작: 첫 번째 상품 UID");
        // 첫 번째 상품 UID에 대한 선반 조작 로직을 여기에 추가합니다.
        count; // count를 정수로 변환 (필요한 경우)
        // 예: 모터를 회전시키거나 선반을 조작하는 코드 추가
        // Serial.println("조작할 상품 수량: " + count);
    } else if (uid == config.secondProductUid) {
        Serial.println("[MOTER] 선반 조작 시작: 두 번째 상품 UID");
        // 두 번째 상품 UID에 대한 선반 조작 로직을 여기에 추가합니다.
        count; // count를 정수로 변환 (필요한 경우)
        // 예: 모터를 회전시키거나 선반을 조작하는 코드 추가
        // Serial.println("조작할 상품 수량: " + count);
    } else {
        Serial.println("[MOTER] 유효하지 않은 상품 UID: " + uid);
    }

    // 완료 요청 함수 호출
    sendEndWorkingRequest(uid);
    
}

// [MOTER-2] RFID 카트를 들어올리는 함수입니다.
void upRfidCard(const String& uid) {
    Serial.println("[MOTER] RFID 카드 들어올리기 시작: UID = " + uid);
    if (uid == config.firstProductUid) {
        Serial.println("[MOTER] RFID 카드 들어올리기 시작: 첫 번째 상품 UID");
        
    } else if (uid == config.secondProductUid) {
        Serial.println("[MOTER] RFID 카드 들어올리기 시작: 두 번째 상품 UID");
        
    }
}

// [MOTER-3] RFID 카트를 내리는 함수입니다.
void downRfidCard(const String& uid) {
    Serial.println("[MOTER] RFID 카드 내려놓기 시작: UID = " + uid);
    
    if (uid == config.firstProductUid) {
        Serial.println("[MOTER] RFID 카드 내려놓기 시작: 첫 번째 상품 UID");

    } else if (uid == config.secondProductUid) {
        Serial.println("[MOTER] RFID 카드 내려놓기 시작: 두 번째 상품 UID");
        
    }
}