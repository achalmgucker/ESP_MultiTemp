/**
 * Provide MQTT comunications
 */

#include <Arduino.h>
#include "mqttcomm.h"
#include "mqtt_defines.h"

//

void MqttComm::Setup()
{
    _state = MqttState::PREINIT;
    String id = String(ESP.getChipId(), HEX);
    _cltId = "MultiTemp-" + id;
    _top_cmd = TOPIC_CMD;
    _top_stat = TOPIC_STATUS;
    _top_tele = TOPIC_TELE;
    _top_cmd.replace("%C", id.c_str());
    _top_stat.replace("%C", id.c_str());
    _top_tele.replace("%C", id.c_str());
    _top_connstat = _top_tele + "CONN";
    _top_sensor = _top_tele + "SENSOR";

    Serial.println("MQTT Client initialisiert.");
    // Prepare MQTT communication
    _clt = new PubSubClient(MQTTSERVER, MQTTPORT, _net->GetClient());
    _clt->setKeepAlive(15);

    // mClt.onConnect(onMqttConnect);
    // mClt.onDisconnect(onMqttDisconnect);
    // mClt.onSubscribe(onMqttSubscribe);
    // mClt.onUnsubscribe(onMqttUnsubscribe);
    // mClt.onMessage(onMqttMessage);
    // mClt.onPublish(onMqttPublish);
    // mClt.setServer(MQTTSERVER, MQTTPORT);
    // mClt.setCredentials(MQTTUSER, MQTTPASS);
    // mClt.setClientId(_cltId.c_str());
    // mClt.setWill(_top_connstat.c_str(), 0, true, "OFFLINE", 7);
}

void MqttComm::Loop()
{
    switch (_state)
    {
        case MqttState::PREINIT:
            _state = MqttState::DISCONNECTED;
            break;
        case MqttState::DISCONNECTED:
            if (_net->IsConnected())
            {
                Serial.println("Try connection");
                if (_clt->connect(
                            _cltId.c_str(),
                            MQTTUSER, MQTTPASS,
                            _top_connstat.c_str(),
                            0, true,
                            "OFFLINE",
                            false))
                {
                    _state = MqttState::CONNECTED;
                    Serial.println("Connected to MQTT broker.");
                    _clt->publish(_top_connstat.c_str(), "ONLINE");
                    break;
                }
            }
            break;
        case MqttState::CONNECTED:
            if (!_net->IsConnected() || !_clt->connected())
            {
                _state = MqttState::DISCONNECTED;
                _clt->disconnect();
                Serial.println("Disconnected from MQTT broker.");
                break;
            }
            _clt->loop();
            if (_do_publish)
            {
                _clt->publish(_top_sensor.c_str(), _to_publish.c_str());
                _do_publish = false;
            }
            break;
        default:
            _state = MqttState::PREINIT;
            break;
    }
}

void MqttComm::PublishData(String data)
{
    if (_do_publish)
        return;
    
    _to_publish = data;
    _do_publish = true;
}


// *******************************
//   Callbacks to MQTT functions
// *******************************

void MqttComm::onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  // uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
  // Serial.print("Subscribing at QoS 2, packetId: ");
  // Serial.println(packetIdSub);
  // mqttClient.publish("test/lol", 0, true, "test 1");
  // Serial.println("Publishing at QoS 0");
  // uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
  // Serial.print("Publishing at QoS 1, packetId: ");
  // Serial.println(packetIdPub1);
  // uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  // Serial.print("Publishing at QoS 2, packetId: ");
  // Serial.println(packetIdPub2);
}

// void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
//   Serial.println("Disconnected from MQTT.");

//   if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
//     Serial.println("Bad server fingerprint.");
//   }

//   if (WiFi.isConnected()) {
//     mqttReconnectTimer.once(2, connectToMqtt);
//   }
// }

// void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
//   Serial.println("Subscribe acknowledged.");
//   Serial.print("  packetId: ");
//   Serial.println(packetId);
//   Serial.print("  qos: ");
//   Serial.println(qos);
// }

// void onMqttUnsubscribe(uint16_t packetId) {
//   Serial.println("Unsubscribe acknowledged.");
//   Serial.print("  packetId: ");
//   Serial.println(packetId);
// }

// void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
//   Serial.println("Publish received.");
//   Serial.print("  topic: ");
//   Serial.println(topic);
//   Serial.print("  qos: ");
//   Serial.println(properties.qos);
//   Serial.print("  dup: ");
//   Serial.println(properties.dup);
//   Serial.print("  retain: ");
//   Serial.println(properties.retain);
//   Serial.print("  len: ");
//   Serial.println(len);
//   Serial.print("  index: ");
//   Serial.println(index);
//   Serial.print("  total: ");
//   Serial.println(total);
// }

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}
