/* main.cpp */
#define nDeltaAllFilters 10000
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using WebServer = ESP8266WebServer;
#include <Wire.h>
#include "Config.h"
// Создаем объект веб-сервера
extern WebServer   server;
// параметры WiFi
#if defined(LASER)
#define WIFI_SSID "mitinskaia2"
#define WIFI_PASSWORD "14rfvgd90"
#endif
#if defined(DIR620)
#define WIFI_SSID "DIR-620"
#define WIFI_PASSWORD "123454321"
#endif
#if defined(RedmiNote)
#define WIFI_SSID "Redmi"
#define WIFI_PASSWORD "****"
#endif
const char* ssid = WIFI_SSID;     // Замените на имя вашей WiFi сети
const char* password = WIFI_PASSWORD; // Замените на пароль вашей WiFi сети

#include "Evgen/SD_OpenTherm.hpp"
#include "SWITCH/JHM1200.h"
#include "fsaindcntrl10/FDgtFilter.h"
/************************************/
extern void setup_read_config(void);
extern void setup_web_common(void);
extern void loop_web(void);
/************************************/
class SD_Termo SmOT;

void loop2(void);

int LedSts = 0; //LOW
JHM1200 sensorPressure("Pressure");
FDgtFilter DgtFilterPressure("DgtFilterPressure");

void setup() {
  DgtFilterPressure.pVarX = &sensorPressure.press_bar;  // значение от датчика давления  
  DgtFilterPressure.pVarY = &sensorPressure.fPressure;  // значение после фильтра
  DgtFilterPressure.nN = 50;               // длина фильтра
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LedSts);   // Turn the LED on (Note that LOW is the voltage level
  
  delay(2);
  Serial.begin(115200);
  Serial.println();
  Serial.println(IDENTIFY_TEXT);
  Serial.printf("Vers %d.%d.%d\n",SmOT.Vers, SmOT.SubVers,SmOT.SubVers1);
  LedSts=1;
  digitalWrite(LED_BUILTIN, LedSts);   

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("IP address: " + WiFi.localIP().toString());
  void setup_web_common_onconnect();
  setup_web_common_onconnect();
  setup_web_common();

  if (sensorPressure.begin()) {
      Serial.println("JHM1200 6Bar Pressure Sensor Ready");
      Serial.println("Pressure (kPa)\tPressure (Bar)");
  }

}
////////////////////////////////////////////////////////

static unsigned long lastUpdateFilters = 10000;
#define OT_CICLE_TIME 300
bool bIfLoop2 = false;
void loop(void)
{
  loop_web();

  sensorPressure.run();
  if (millis() - lastUpdateFilters > nDeltaAllFilters) {  // Обновляем каждые 10 мсекунд
    lastUpdateFilters = millis();
    if (sensorPressure.bIfPressureSensors) {
      DgtFilterPressure.run();         // работает фильтр давления 
#if defined(ARDUINO_ARCH_ESP8266)
  #if defined(IF_SWITCHPRESSURESENSOR)
      if (!sensorPressure.bIfError) {
          Serial.printf("%.2f kPa\t\t%.3f Bar, temperature = %.3f C\n", sensorPressure.press_kpa, sensorPressure.fPressure, sensorPressure.temp_c);
      } else {
          Serial.println("Measurement error");
      }
  #else    
      if (sensorPressure.readPressure()) {
        Serial.printf("%.2f kPa\t\t%.3f Bar, temperature = %.3f C\n", 
        sensorPressure.press_kpa, sensorPressure.fPressure, sensorPressure.temp_c);
      } else {
        Serial.println("Measurement error");
      }
  #endif      
#endif      
    }
  }

}

void loop2(void)
{   
    static unsigned long  t0=0; // t1=0;
    unsigned long t, dt;
     t = millis();
     dt = t - t0;
     if(LedSts) //быстро моргаем раз в мсек
     {  if(dt > 2)
        { LedSts = (LedSts+1)&0x01;
          digitalWrite(LED_BUILTIN, LedSts);   
          t0 = t;
        }
     } else {
        int wt = 500;
        if(dt > (unsigned long)wt)
        { LedSts = (LedSts+1)&0x01;
          digitalWrite(LED_BUILTIN, LedSts);   
          t0 = t;   
        }
     }
    SmOT.loop();  // сохраняем параметры
    loop_web();
}
