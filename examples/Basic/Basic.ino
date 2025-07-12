#include <WiFiManager.h>

WiFiManager wifi;

void setup() {
  Serial.begin(115200);
  wifi.begin();
}

void loop() {
  wifi.loop();
}