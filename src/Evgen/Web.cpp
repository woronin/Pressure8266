/* Web.cpp  UTF-8  */
#include "Config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using WebServer = ESP8266WebServer;
// Создаем объект веб-сервера
WebServer   server;
#include "SmartDevice.hpp"
#include "SD_OpenTherm.hpp"
extern void mqtt_setup(void);
extern void mqtt_loop(void);
extern void mqtt_start(void);
extern  SD_Termo SmOT;
int WiFiDebugInfo[10] ={0,0,0,0,0, 0,0,0,0,0};

void setup_web_common(void)
{
//  Serial.println();
//  Serial.println("setup_web_common");

ESP8266WebServer&  webServer = server;

#if defined (IF_WEBSITE)
  void handleRootOsc();
  webServer.on("/", handleRootOsc);                        // менеджер базовой страницы сайта
    // Обработка URL /data
  void handlePressureValues();
  webServer.on("/current-pressure", handlePressureValues);    // периодическое обновление данных сайта
  void handleSettings();
  webServer.on("/settings", handleSettings);               // настройки
    void handleGPIO();
  webServer.on("/io", handleGPIO);                         // обработка событий от кнопок
  webServer.begin();
#else
  webServer.on("/", onRoot);  // Register the root page redirector.
#endif  
//  Serial.println("Web server started:" +WiFi.localIP().toString());
  Serial.printf("WL_CONNECTED = %d", WL_CONNECTED);
  if (WiFi.status() != WL_CONNECTED)  {
    Serial.println(F("WiFi Not connected"));
//    WiFi.setAutoReconnect(true);
  }  
}

int setup_web_common_onconnect(void) {
  static int init = 0;

  //Serial.printf("setup_web_common_onconnect init %d\n", init);

  Serial.print(F("WiFi connected, IP address: "));
  Serial.println(WiFi.localIP());
  sprintf(SmOT.LocalUrl,"http://%s", WiFi.localIP().toString().c_str());
  Serial.printf("WiFi mode = %d\n", WiFi.getMode());
  if(init)
    return 1;

   WiFi.setAutoReconnect(true);

   {
const char*  const _ntp1 = "europe.pool.ntp.org";
const char*  const _ntp2 = "pool.ntp.org";

    configTzTime("UTC0", _ntp1 ,_ntp2);
}
  init  = 1;
  return 0;
}

float mRSSi = 0.;
int WiFists = -1;

int sRSSI = 0;
int razRSSI = 0;
extern int LedSts; 

void loop_web()
{ 
  int rc,  dt;
  static unsigned long t0=0; // t1=0;

  server.handleClient();

  rc = WiFi.status();
  { 
    static int oldstatus=-1, oldmode=-1, needStopAP=0 ;
    static long t0 = 0;
    int mode = WiFi.getMode();
    int ch = WiFi.channel();

    if((rc != oldstatus) || mode != oldmode)
    {
        Serial.printf("WiFi: status=%i mode = %d chanel=%d\n", rc,  mode, ch);
        if(rc == WL_CONNECTED &&  (oldstatus == WL_IDLE_STATUS || oldstatus == WL_DISCONNECTED ||  oldstatus == WL_NO_SSID_AVAIL))
        {   Serial.printf("WiFi status chage to connected");
          needStopAP = 1;
            t0 = millis();
        }
        oldmode = mode;
        oldstatus = rc;
    } else if(needStopAP) {
      if(millis()-t0 > 10000)
      {   needStopAP = 0; }
    }
  }
  if(rc != WiFists) { 
    if(rc == WL_CONNECTED)
    {  LedSts = 0;
    } else {
      LedSts = 1;
    }

    if( rc >=0 && rc <=7)
        WiFiDebugInfo[rc]++;
    WiFists = rc;
  }

  if(rc ==  WL_CONNECTED)
  {  dt = millis() - t0;
     if(dt > 10000)
     {   t0 = millis();
         razRSSI++;
         sRSSI += WiFi.RSSI();
         if(razRSSI > 6*10)
         {  mRSSi =  float(sRSSI)/float(razRSSI);
            razRSSI = 0;
            sRSSI = 0;
         }
    }
  }
  if(rc ==  WL_CONNECTED && (SmOT.useMQTT== 0x03))
         mqtt_loop();

}
