#include "WiFiManager.h"
#include <EEPROM.h>

WiFiManager::WiFiManager() : server(80) {}

void WiFiManager::begin() {
  EEPROM.begin(512);
  loadCredentials();

  WiFi.begin(ssid.c_str(), password.c_str());
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    startAP();
  }
}

void WiFiManager::loop() {
  server.handleClient();
}

void WiFiManager::startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP_Config");

  server.on("/", std::bind(&WiFiManager::handleRoot, this));
  server.on("/save", std::bind(&WiFiManager::handleSave, this));
  server.begin();
}

// ===== IMPROVED WEBPAGE WITH CSS & WIFI SCANNER =====
void WiFiManager::handleRoot() {
  // Scan for nearby WiFi networks
  int numNetworks = WiFi.scanNetworks();
  
  // Generate HTML with inline CSS
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: 'Arial', sans-serif;
      background: #f5f5f5;
      margin: 0;
      padding: 20px;
      color: #333;
    }
    .container {
      background: white;
      max-width: 400px;
      margin: 30px auto;
      padding: 25px;
      border-radius: 8px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    h1 {
      color: #2c3e50;
      text-align: center;
      margin-bottom: 25px;
    }
    label {
      display: block;
      margin: 15px 0 5px;
      font-weight: bold;
    }
    select, input[type="password"] {
      width: 100%;
      padding: 12px;
      margin-bottom: 15px;
      border: 1px solid #ddd;
      border-radius: 4px;
      font-size: 16px;
      box-sizing: border-box;
    }
    input[type="submit"] {
      background: #3498db;
      color: white;
      border: none;
      padding: 12px;
      width: 100%;
      border-radius: 4px;
      font-size: 16px;
      cursor: pointer;
      transition: background 0.3s;
    }
    input[type="submit"]:hover {
      background: #2980b9;
    }
    .status {
      text-align: center;
      margin-top: 20px;
      color: #7f8c8d;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>WiFi Configuration</h1>
    <form action="/save">
      <label for="ssid">Select your WiFi:</label>
      <select id="ssid" name="ssid" required>
        <option value="" disabled selected>Choose a network...</option>
  )";

  // Add scanned networks to dropdown
  for (int i = 0; i < numNetworks; i++) {
    html += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + " dBm)</option>";
  }

  // Close the form
  html += R"(
      </select>
      <label for="pass">Password:</label>
      <input type="password" id="pass" name="pass" placeholder="Enter WiFi password">
      <input type="submit" value="Connect">
    </form>
    <div class="status">Device IP: )";
  html += WiFi.softAPIP().toString();
  html += "</div></div></body></html>";

  server.send(200, "text/html", html);
}


void WiFiManager::handleSave() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    ssid = server.arg("ssid");
    password = server.arg("pass");
    saveCredentials(ssid, password);
    server.send(200, "text/html", "<h1 style='text-align:center;color:#27ae60;'>Credentials saved! Rebooting...</h1>");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Error: Missing SSID or password");
  }
}

void WiFiManager::loadCredentials() {
  ssid = "";
  password = "";
  for (int i = 0; i < 32; ++i) {
    char c = EEPROM.read(i);
    if (c == 0) break;
    ssid += c;
  }
  for (int i = 32; i < 96; ++i) {
    char c = EEPROM.read(i);
    if (c == 0) break;
    password += c;
  }
}

void WiFiManager::saveCredentials(String s, String p) {
  for (int i = 0; i < 96; ++i) EEPROM.write(i, 0); // Clear
  for (int i = 0; i < s.length(); ++i) EEPROM.write(i, s[i]);
  for (int i = 0; i < p.length(); ++i) EEPROM.write(32 + i, p[i]);
  EEPROM.commit();
}