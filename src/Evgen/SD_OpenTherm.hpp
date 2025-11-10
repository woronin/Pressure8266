/* SD_OpenTherm.hpp */
#ifndef SD_OPENTHERM
#define SD_OPENTHERM

#include "SmartDevice.hpp"

class SD_Termo:public SmartDevice
{
public:
  float dhw_t;   // DHW temperature (°C)
  byte need_write_f; 

  
#if MQTT_USE
  byte useMQTT;  //0 = not use, 1 use but not setup, 0x3 - use & setup
  byte stsMQTT;
 //40+40+4+40+20+40=  184 (+110)
  char MQTT_server[80];
  char MQTT_topic[40];
  int MQTT_interval; //sec
  char MQTT_user[40];
  char MQTT_pwd[20];
  char MQTT_devname[40];
  unsigned short MQTT_port;  /* MQTT port, default 1883 */
  int MQTT_need_report;
#endif //MQTT_USE
  int start_sts; //1 - start state, need ask server for last I and U0(?),  &0x02 - OT start log, 0 - not start


  SD_Termo(void)
  {	  

      dhw_t = 0.;
#if MQTT_USE
//      useMQTT = 0;
#if defined(LASER)
      useMQTT = 0x3;
      stsMQTT = 0;

      strcpy(MQTT_server,"192.168.0.132");
      strcpy(MQTT_topic,"PS_KY-IIC");
      strcpy(MQTT_devname,"Pressure Sensor");
      strcpy(MQTT_user,"umki2");
      strcpy(MQTT_pwd,"12345");
      MQTT_interval = 10; //sec
      MQTT_need_report = 0;
      MQTT_port = 1883;
/*      
      strcpy(MQTT_server,"192.168.0.181");
      strcpy(MQTT_topic,"PS");
      strcpy(MQTT_devname,"PressureSensor");
      strcpy(MQTT_user,"umki");
      strcpy(MQTT_pwd,"1234");
      MQTT_interval = 10; //sec
      MQTT_need_report = 0;
      MQTT_port = 1883;      
     
      strcpy(MQTT_server,"80.237.33.119");
      strcpy(MQTT_topic,"PS_KY-IIC");
      strcpy(MQTT_devname,"Pressure Sensor");
      strcpy(MQTT_user,"qq_umki");
      strcpy(MQTT_pwd,"54321");
      MQTT_interval = 10; //sec
      MQTT_need_report = 0;
      MQTT_port = 12883;
*/
      
#endif
#if defined(DIR620)
      useMQTT = 0x3;
      stsMQTT = 0;
      strcpy(MQTT_server,"80.237.33.118");
      strcpy(MQTT_topic,"DAV");
      strcpy(MQTT_devname,"Pressure Sensor");
      strcpy(MQTT_user,"mq_umki");
      strcpy(MQTT_pwd,"54321");
      MQTT_interval = 10; //sec
      MQTT_need_report = 0;
      MQTT_port = 12883;
#endif

#endif     
    start_sts = 1;
    need_write_f = 0;
  }
  void loop(void);
  int Write_data_fs(char *path, uint8_t *dataBuff, int len, int mode);
  int Read_data_fs(char *path, uint8_t *dataBuff, int len, int &rlen, int mode);
  int Read_mqtt_fs(void);
  int Write_mqtt_fs(void);
  int Write_ot_fs(void);

};

#endif // SD_OPENTHERM
