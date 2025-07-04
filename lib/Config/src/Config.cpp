#include "Config.h"
#include <Preferences.h>

Preferences prefs;
Config config;

void Config::load() {
  prefs.begin("settings", true);

  ssid            = prefs.getString("ssid", "");
  password        = prefs.getString("password", "");

  serverIP        = prefs.getString("server_ip", "oxxultus.kro.kr");
  serverPort      = prefs.getInt("server_port", 8080);
  innerPort       = prefs.getInt("inner_port", 8082);
  localIP         = prefs.getString("localIP", ""); 

  serialBaudrate  = prefs.getInt("baudrate", 115200);
  serial2Baudrate = prefs.getInt("baudrate2", 9600);

  checkWorkingList = prefs.getString("cwl", "/check/working-list?uid=");
  endWorkingList   = prefs.getString("ewl", "/end/working-list?uid=");

  prefs.end();
}

void Config::save() {
  prefs.begin("settings", false);

  prefs.putString("ssid", ssid);
  prefs.putString("password", password);

  prefs.putString("server_ip", serverIP);
  prefs.putInt("server_port", serverPort);
  prefs.putInt("inner_port", innerPort);
  prefs.putString("localIP", localIP);

  prefs.putInt("baudrate", serialBaudrate);
  prefs.putInt("baudrate2", serial2Baudrate);

  prefs.putString("cwl", checkWorkingList);
  prefs.putString("ewl", endWorkingList);  

  prefs.end();
}