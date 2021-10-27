/**
 * Provide a network connection via WiFi (STA or AP)
 */

#include "network.h"
#include "wlan_defines.h"

const unsigned long CONNECT_TEST_MIN_TIME_MS = 1000;

Network::Network()
    : _state(NetworkState::PREINIT),
      _prevTime(0),
      _connected(false)
{
}

Network::~Network()
{
    //
}

bool Network::IsConnected()
{
    return _connected;
}

String Network::GetSsid()
{
    return _connected ? _currSsid : "";
}

IPAddress Network::GetIpAddr()
{
    return _connected ? _currIpAddr : INADDR_NONE;
}


/**
 * Initial called setup.
 */
void Network::Setup()
{
    // Set the access point definition
#ifdef SSID1
    _wifiMulti.addAP(SSID1, PASSWORD1);
#endif
#ifdef SSID2
    _wifiMulti.addAP(SSID2, PASSWORD2);
#endif
#ifdef SSID3
    _wifiMulti.addAP(SSID3, PASSWORD3);
#endif
#ifdef MYSSID
    WiFi.setHostname(MYSSID);
#endif
}

/**
 * Periodically called worker.
 */
void Network::Loop()
{
    unsigned long currTime = millis();
    switch (_state)
    {
        case NetworkState::PREINIT:
            ArduinoOTA.setHostname(OTAHOSTNAME);
            ArduinoOTA.setPassword(OTAPASSWORD);
            ArduinoOTA.onStart([]()
                               { Serial.println("Start"); });
            ArduinoOTA.onEnd([]()
                             { Serial.println("End"); });
            ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });

            ArduinoOTA.onError([](ota_error_t error)
                               {
                                   Serial.printf("Error[%u]: ", error);
                                   if (error == OTA_AUTH_ERROR)
                                       Serial.println("Auth Failed");
                                   else if (error == OTA_BEGIN_ERROR)
                                       Serial.println("Begin Failed");
                                   else if (error == OTA_CONNECT_ERROR)
                                       Serial.println("Connect Failed");
                                   else if (error == OTA_RECEIVE_ERROR)
                                       Serial.println("Receive Failed");
                                   else if (error == OTA_END_ERROR)
                                       Serial.println("End Failed");
                               });
            ArduinoOTA.begin();
            Serial.println("OTA ready");

            _prevTime = currTime;
            _state = NetworkState::DISCONNECTED;
            break;
        case NetworkState::DISCONNECTED:
            if ((currTime - _prevTime) > CONNECT_TEST_MIN_TIME_MS)
            {
                if (_wifiMulti.run(8000) == WL_CONNECTED)
                {
                    _currSsid = WiFi.SSID();
                    _currIpAddr = WiFi.localIP();
                    _connected = true;
                    _state = NetworkState::CONNECTED;
                    if (!MDNS.begin("GREENHOUSE"))
                    { // Start the mDNS responder for GREENHOUSE.local
                        Serial.println("Error setting up MDNS responder!");
                    }
                    Serial.print("SSID: ");
                    Serial.println(_currSsid);
                    Serial.print("IP: ");
                    Serial.println(_currIpAddr);
                }
                else
                    _prevTime = currTime;
            }
            break;
        case NetworkState::CONNECTED:
            if (_wifiMulti.run() != WL_CONNECTED)
            {
                _connected = false;
                _state = NetworkState::DISCONNECTED;
                _prevTime = currTime;
                MDNS.end();
                break;
            }
            ArduinoOTA.handle();
            MDNS.update();
            break;
        default:
            break;
    }
}

