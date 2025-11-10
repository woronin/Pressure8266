#include "JHM1200.h"

JHM1200::JHM1200(String strN):LFsaAppl(strN) { }

static unsigned long lastRead = 0;
static unsigned int nTimeT10 = 10;
static unsigned int nTimeT500 = 500;
int JHM1200::x1() { return Wire.endTransmission() != 0; }
int JHM1200::x2() { return Wire.requestFrom(JHM1200_ADDR, 6) != 6; }
int JHM1200::x3() { return (millis() - lastRead) >= nTimeT10; }
int JHM1200::x4() { return (millis() - lastRead) >= nTimeT500; }

void JHM1200::y1() { 
    Wire.beginTransmission(JHM1200_ADDR);
    Wire.write(0xAC);
}

void JHM1200::y2() { 
    uint8_t buffer[6];
    for (int i = 0; i < 6; i++) {
        buffer[i] = Wire.read();
    }

    // Convert 24-bit pressure data
    uint32_t press_raw = ((uint32_t)buffer[1] << 16) | 
                        ((uint32_t)buffer[2] << 8) | 
                        buffer[3];
    // Convert temperature (16-bit)  
    uint16_t temp_raw = ((uint16_t)buffer[4] << 8) | buffer[5];

    // Convert to pressure (0-600 kPa for 6Bar sensor)
    double pa= 1.861, pb=-1.6045 , ta= -0.7206, tb= 39.73;
    press_kpa = ((double)press_raw / 16777216.0) * 1000.0;
    press_bar = pa * (press_kpa / 100.0) + pb ;
    temp_c =  ta * (((double)temp_raw / 65536.0) * 165.0 - 71.0) + tb;    
}

void JHM1200::y3() { lastRead = millis(); }   

bool JHM1200::readPressure() 
{
// Send measurement command
    y1();
    if (x1()) return false;
    // Wait for conversion
    delay(10);
    // Read data
    if (x2()) return false;
    y2();
    delay(500);
    return true;
/*
    if (nState==0) { y1(); nTmpState=1;} 
    if (nState==1 && x1()) { nTmpState=0; bIfError = true; } 
    if (nState==1 && !x1()) { y3();  nTmpState=2; } 
    if (nState==2 && !x3()) { y3(); } 
    if (nState==2 && x3() && x2()) { nTmpState=0; bIfError = true; } 
    if (nState==2 && x3() && !x2()) { y2(); y3(); nTmpState=3; } 
    if (nState==2 && !x4()) { y3(); } 
    if (nState==2 && x4()) { nTmpState=0; } 
    bIfError = false; 
*/    
}

void JHM1200::run() {
    if (nState==0) { y1(); nTmpState=1;} 
    if (nState==1 && x1()) { nTmpState=0; bIfError = true; } 
    if (nState==1 && !x1()) { y3();  nTmpState=2; } 
    if (nState==2 && !x3()) { y3(); } 
    if (nState==2 && x3() && x2()) { nTmpState=0; bIfError = true; } 
    if (nState==2 && x3() && !x2()) { y2(); y3(); nTmpState=3; } 
    if (nState==3 && !x4()) { y3(); } 
    if (nState==3 && x4()) { nTmpState=0; } 
    bIfError = false; 
    if (nTmpState != nState) { nState = nTmpState; }
}

bool JHM1200::readPressure(float &pressure_kpa, float &pressure_bar, float &temperature_c) 
{
    // Send measurement command
    Wire.beginTransmission(JHM1200_ADDR);
    Wire.write(0xAC);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    // Wait for conversion
    delay(10);

    // Read data
    if (Wire.requestFrom(JHM1200_ADDR, 6) != 6) {
        return false;
    }

    uint8_t buffer[6];
    for (int i = 0; i < 6; i++) {
        buffer[i] = Wire.read();
    }

    // Convert 24-bit pressure data
    uint32_t press_raw = ((uint32_t)buffer[1] << 16) | 
                        ((uint32_t)buffer[2] << 8) | 
                        buffer[3];
    // Convert temperature (16-bit)  
    uint16_t temp_raw = ((uint16_t)buffer[4] << 8) | buffer[5];

    // Convert to pressure (0-600 kPa for 6Bar sensor)
    pressure_kpa = ((double)press_raw / 16777216.0) * 1000.0;
    pressure_bar = pressure_kpa / 100.0;

    temperature_c = ((double)temp_raw / 65536.0) * 165.0 - 71.0;

    delay(500);
    return true;
}
