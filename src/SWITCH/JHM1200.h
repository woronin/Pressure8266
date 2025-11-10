#include <Wire.h>
#include <LFsaAppl.h>

#define JHM1200_ADDR 0x78
class JHM1200:public LFsaAppl  
{
public:
    JHM1200(String strN);
    ~JHM1200() {};
    void run();
    bool bIfPressureSensors{true};
    float fPressure{0};         // давление 
    float press_kpa{0}, press_bar{0}, temp_c{0};
    bool begin() {
        Wire.begin();
        return true;
    }

    bool readPressure();
    bool readPressure(float &pressure_kpa, float &pressure_bar, float &temperature_c);
    bool bIfError{false};

protected:
    int x1(); int x2(); int x3(); int x4();
    void y1(); void y2(); void y3();
};
