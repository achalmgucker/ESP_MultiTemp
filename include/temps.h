
#ifndef __TEMPS_H__
#define __TEMPS_H__

#include <DallasTemperature.h>

struct SingleTemp
{
    int port;
    DeviceAddress addr;
    float temp;
    bool active;
};

extern SingleTemp Temps[6];

#endif
