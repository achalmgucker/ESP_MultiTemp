/**
 * Provide a network connection via WiFi (STA or AP)
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

class Network
{
public:
    /**
     * Standardkonstruktor.
     */
    Network();

    /**
     * Destruktor.
     */
    ~Network();

    /**
     * Einmalige Vorbereitung.
     */
    void Setup();

    /**
     * Regelmäßiger Aufruf.
     */
    void Loop();

    /**
     * Query, if network is connected
     */
    bool IsConnected();

    String GetSsid();
    IPAddress GetIpAddr();

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


};
