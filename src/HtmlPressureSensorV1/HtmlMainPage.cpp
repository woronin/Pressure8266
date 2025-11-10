#include "Config.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using WebServer = ESP8266WebServer;
extern WebServer   server;

#if defined(IF_PRESSURESENSOR)
#include "SWITCH/JHM1200.h"
extern JHM1200 sensorPressure;
#endif

String getPageTemplate(String title, String content);

// Навигационное меню
String getNavigationMenu() {
  String strRet = R"(
    <div class="nav-menu">)";
#if defined (AUTO_CONNECT)
    strRet += AUTOCONNECT_LINK(BAR_24);
#endif
    strRet += R"(</div>)";
  return strRet;
}
  
// Общий шаблон страницы
String getPageTemplate(String title, String content) {
  String strRet = R"=====(
    <!DOCTYPE html>
    <html>
    <head>
        <title>ESP32 - )=====" + title + R"=====(</title>
        <meta http-equiv="Content-type" content="text/html; charset=utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
    </head>
    <body>
        <header>
        <h1>ESP8266 Датчик давления</h1>)=====";
        strRet += R"=====(<div class="info">IP: )=====" + WiFi.localIP().toString() + R"=====( | Update: 1s</div>
        </header>
        )=====" + getNavigationMenu() + R"=====(
        <div class="content">
        )=====" + content + R"=====(
        </div>)=====";
        strRet += R"=====(</body>
    </html>
    )=====";
    return strRet;
}
// 1. Запускается при сбросе сайта. 2. Устанавливает начальну страницу сайта 
void handleRootOsc() {
  ESP8266WebServer&  webServer = server;
  webServer.sendHeader("Location", "/settings");
  webServer.send(302, "text/plain", "Redirecting to Oscilloscope");
}

void handleSettings() {
  ESP8266WebServer&  webServer = server;
  String content = R"(
    <style>
      html { font-family: Helvetica; display: inline-block; 
        margin: 0px auto; text-align: center;
      }
      p {font-size: 24px;color: #444444;margin-bottom: 10px;}
      .button {
        display:inline-block; border-radius:7px; background:#73ad21;
        margin:0 10px 0 10px; padding:10px 20px 10px 20px; text-decoration:none;
        color:#000000;
      }
    </style>

    <div class="settings-panel">
      <div class="sensor-values">
        <div class="value-box">
          <div class="value-title">Давление</div>
          <div class="value" id="pressureValue">0.0 МПа</div>
          <div class="value-title">Температура</div>
          <div class="value" id="temperatureValue">0.0 °C</div>
        </div>
      </div>
    </div>
    <script>
    function updateValues() {
        fetch('/current-pressure')
          .then(response => response.json())
          .then(data => {
            document.getElementById('pressureValue').textContent = data.pressure.toFixed(2) + ' МПа';
            document.getElementById('temperatureValue').textContent = data.temperature.toFixed(2) + ' °C';
          });
      }
      
      // Обновляем значения каждую секунду
      setInterval(updateValues, 200);
      updateValues();
    </script>
  )";
  content += String(F("<p>"));
  if (sensorPressure.bIfPressureSensors) { content += String(F("<a class=\"button\" href=\"/io?v=106\">Измер.давления=ON</a>")); }
  else { content += String(F("<a class=\"button\" href=\"/io?v=107\">Измер.давления=OFF</a>")); }
  content += String(F("</p>"));
  webServer.send(200, "text/html", getPageTemplate("Settings", content));
}

// периодическое обновление данных датчика давения
void handlePressureValues() {
  ESP8266WebServer&  webServer = server;
  StaticJsonDocument<200> doc;
#if defined(IF_TEST_APP)
  sensorPressure.fPressure = 0.96;
  sensorPressure.temp_c = 77.7;
#endif  
  doc["pressure"] = sensorPressure.fPressure;
  doc["temperature"] = sensorPressure.temp_c;

  String response;
  serializeJson(doc, response);
  webServer.send(200, "application/json", response);
}
// обработка нажатий кнопки включения/отключения давления на веб-сайте
void handleGPIO() 
{
  ESP8266WebServer&  webServer = server;
  String param = webServer.arg("v");
  int n = param.toInt();
  if (9<n && n<30) {
    void sendRedirect(String uri);
    sendRedirect("/controlPanel");
  }
  else {
    if (webServer.arg("v") == "106") { sensorPressure.bIfPressureSensors = false; }
    if (webServer.arg("v") == "107") { sensorPressure.bIfPressureSensors = true; }
    void sendRedirect(String uri);
    sendRedirect("/settings");
  }
}

void sendRedirect(String uri) 
{
  ESP8266WebServer&  webServer = server;
  webServer.sendHeader("Location", uri, true);
  webServer.send(302, "text/plain", "");
  webServer.client().stop();
}
