#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

class WiFiManager {
  public:
    WiFiManager();
    void begin();
    void loop();

  private:
    void startAP();
    void handleClient();
    void handleRoot();
    void handleSave();
    void loadCredentials();
    void saveCredentials(String ssid, String pass);

    ESP8266WebServer server;
    String ssid;
    String password;
};

#endif