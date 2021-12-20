/**
 * Provide a network connection via WiFi (STA or AP)
 */

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <Arduino.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#include "worker.h"

class Network : public Worker
{
public:
    /**
     * Default construktor.
     */
    Network();

    /**
     * Destructor.
     */
    ~Network();

    /**
     * One-time setup.
     */
    void Setup() override;

    /**
     * Periodic workload.
     */
    void Loop() override;

    /**
     * Query, if network is connected
     */
    bool IsConnected();

    String GetSsid();
    IPAddress GetIpAddr();
    WiFiClient &GetClient() { return _client; }

    unsigned long GetTime();

    enum class NetworkState
    {
        PREINIT = 0,
        DISCONNECTED,
        CONNECTED,
    };


private:
    ESP8266WiFiMulti _wifiMulti;
    NetworkState _state;
    unsigned long _prevTime;
    bool _connected;
    String _currSsid;
    IPAddress _currIpAddr;
    WiFiClient _client;

};
#endif
