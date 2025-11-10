/* SD_MQTT.cpp */

#include <Config.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using WiFiWebServer = ESP8266WebServer;
#define FORMAT_ON_FAIL

#include <time.h>
#include "Smart_Config.h"
#include "SmartDevice.hpp"
#include "SD_OpenTherm.hpp"

#if MQTT_USE 

#include <ArduinoHA.h>
#include <PubSubClient.h>

void mqtt_start(void);
void mqtt_loop(void);
void mqtt_setup(void);

char SmartDevice::LocalUrl[24] = "";

WiFiClient espClient;
PubSubClient client(espClient);
extern  SD_Termo SmOT;

/*******************************************************************************/
HADevice device;
HAMqtt mqtt(espClient, device,12);
const char * temperature_str = "temperature";

#include "SWITCH/JHM1200.h"
extern JHM1200 sensorPressure;

#if defined (IF_PRESSURESENSOR)
HASensor sensorHAPressure(NULL,HANumber::PrecisionP3);
HASwitch swPressure("swPressure");
void onSwitchCommandPressure(bool state, HASwitch* sender)
{
    sensorPressure.bIfPressureSensors = state; 
    sender->setState(state); // report state back to the Home Assistant
//    Serial.printf("sensorPressure.bIfPressureSensors - %d\n", sensorPressure.bIfPressureSensors);
}
#endif
#if defined (IF_TEMPERATURESENSOR)
HASensor sensorHATemperature(NULL,HANumber::PrecisionP3);
#endif

// By default HAHVAC supports only reporting of the temperature.
// You can enable feature you need using the second argument of the constructor.
// Please check the documentation of the HAHVAC class.
unsigned long lastReadAt = millis();
unsigned long lastAvailabilityToggleAt = millis();
bool lastInputState = false;
void OnMQTTconnected(void);
void OnMQTTdisconnected(void);

void onPowerCommand(bool state, HAHVAC* sender) {
  if (state) {
    Serial.println("Power on");
  } else {
    Serial.println("Power off");
  }
}

int statemqtt = -1;
int state_mqtt = -10000;
int attempt_mqtt = 0;

/************************************************************/
void mqtt_setup(void)
{
//   Serial.printf("void mqtt_setup(void)\n");

  bool rc;

  if (WiFi.status() != WL_CONNECTED)  
        return;
   Serial.printf("SmOT.useMQTT = %d\n", SmOT.useMQTT);
   if(SmOT.useMQTT != 0x03) 
      return;

  
  if( mqtt.getDevicesTypesNb_toreg() > mqtt.getDevicesTypesNb())
  {
      Serial.printf("Error! Nb = %d, need be %d\n", mqtt.getDevicesTypesNb(),  mqtt.getDevicesTypesNb_toreg() );
    return;
  }

   device.setUniqueIdStr(SmOT.MQTT_topic);
   device.setName(SmOT.MQTT_topic,SmOT.MQTT_devname); //должно быть static!!
  { static char str[40];
  
    sprintf(str,"%d.%d.%d" , SmOT.Vers,SmOT.SubVers,SmOT.SubVers1);
    device.setSoftwareVersion(str); //должно быть static!!
    device.setConfigurationUrl(SmOT.LocalUrl);// --//--
  }
    device.enableSharedAvailability();
    device.enableLastWill();

    lastReadAt = millis();
    lastAvailabilityToggleAt = millis();
/*********************************/
#if defined (IF_PRESSURESENSOR)
    sensorHAPressure.setAvailability(true);
    sensorHAPressure.setNameUniqueIdStr(SmOT.MQTT_topic,"KY-IIC_Pressure", "agr_pr");
    sensorHAPressure.setDeviceClass(temperature_str); 
    sensorHAPressure.setUnitOfMeasurement("MPa");
#endif    
#if defined (IF_TEMPERATURESENSOR)
    sensorHATemperature.setAvailability(true);
    sensorHATemperature.setNameUniqueIdStr(SmOT.MQTT_topic,"KY-IIC_Temperature", "agr_tm");
    sensorHATemperature.setDeviceClass(temperature_str); 
    sensorHATemperature.setUnitOfMeasurement("°C");
#endif    
 
mqtt.onConnected(OnMQTTconnected);
mqtt.onDisconnected(OnMQTTdisconnected);
SmOT.stsMQTT = 1;
mqtt._mqtt->setSocketTimeout(1); //not work ???


    rc= mqtt.begin(SmOT.MQTT_server, SmOT.MQTT_port, SmOT.MQTT_user, SmOT.MQTT_pwd);
    if(rc == true)
    {  
      Serial.printf("mqtt.begin ok %s:%d %s %s\n", SmOT.MQTT_server, SmOT.MQTT_port, SmOT.MQTT_user, SmOT.MQTT_pwd);
      state_mqtt = mqtt._mqtt->state();
  //    Serial.printf("mqtt_setup():MQTT connected: state: %d; SmOT.stsMQTT = %d\n", state_mqtt, SmOT.stsMQTT);
      SmOT.stsMQTT = 2;
    } else {
      Serial.printf("mqtt.begin false\n");
    }
//*    
#if defined (IF_PRESSURESENSOR)
    swPressure.setIcon("mdi:lightbulb");
    swPressure.setName("On/Off");
    swPressure.onCommand(onSwitchCommandPressure);
    swPressure.setNameUniqueIdStr(SmOT.MQTT_topic,"swPressure", "agr_swPressure");
#endif    
//*/
}

void OnMQTTconnected(void)
{ 
  statemqtt = 1;
//  Serial.printf("OnMQTTconnected %d\n", statemqtt );

}
void OnMQTTdisconnected(void)
{ statemqtt = 0;
//  Serial.printf("OnMQTT disconnected %d\n", statemqtt );
}

void mqtt_start(void)
{
  if(SmOT.stsMQTT == 0)
  {   mqtt_setup();
  } else {
    int rc = true;
    if(rc == true)
    { Serial.printf("(1) mqtt.begin ok %s %s %s\n", SmOT.MQTT_server,SmOT.MQTT_user, SmOT.MQTT_pwd);
      SmOT.stsMQTT = 2;
    } else {
      Serial.printf("(1)mqtt.begin false\n");
    }
  }
}

void mqtt_loop(void)
{ 
  if(SmOT.stsMQTT == 0) {   mqtt_setup(); return; }

  mqtt.loop();

  if(mqtt.isConnected())
  {   
    if(statemqtt != 1)
      Serial.println(F("MQTT connected"));
    statemqtt = 1;
    state_mqtt = mqtt._mqtt->state();
  } 
  else {
    if(statemqtt != 0)
        Serial.println(F("MQTT DiSconnected"));
    statemqtt = 0;
    state_mqtt = mqtt._mqtt->state();
    delay(1000);
    return; // return from   mqtt_loop() if not connected
  }

  char str[80];
  float iii;
  if ((millis() - lastAvailabilityToggleAt) > SmOT.MQTT_interval*1000 || SmOT.MQTT_need_report)
  {   
#if defined(IF_PRESSURESENSOR)
    iii = sensorPressure.fPressure;
    sprintf(str,"%.1f", iii);
    sensorHAPressure.setValue(str);
      delay(100);
#endif    
#if defined(IF_TEMPERATURESENSOR)
    iii = sensorPressure.temp_c;
    sprintf(str,"%.1f", iii);
    sensorHATemperature.setValue(str);
      delay(100);
#endif    
      lastAvailabilityToggleAt = millis();
      SmOT.MQTT_need_report = 0;

  }
#if defined(IF_PRESSURESENSOR)
  swPressure.setState(sensorPressure.bIfPressureSensors, true);
  delay(300);
#endif        

}
/*******************************************************************************/
#endif  //MQTT_USE 
