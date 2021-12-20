/**
 * Provide MQTT comunications
 */

#ifndef __MQTTCOMM_H__
#define __MQTTCOMM_H__

#include <Ticker.h>
// #include <AsyncMqttClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include "network.h"
#include "worker.h"

class MqttComm : public Worker
{
public:
    MqttComm(Network *net)
        : _net(net), _state(MqttState::PREINIT)
        {};
    ~MqttComm() override {};

    void Setup() override;

    void Loop() override;

    WiFiClient *GetClient();

    void PublishData(String data);

    enum class MqttState
    {
        PREINIT = 0,
        DISCONNECTED,
        CONNECTED,
    };

private:
    void onMqttConnect(bool sessionPresent);

private:
    Network     *_net;
    // AsyncMqttClient mClt;
    String _cltId;
    String _top_stat;
    String _top_tele;
    String _top_cmd;
    String _top_connstat;
    String _top_sensor;
    Ticker mqttReconnectTimer;
    MqttState _state;
    PubSubClient *_clt;
    String _to_publish;
    bool _do_publish;
};


#endif
