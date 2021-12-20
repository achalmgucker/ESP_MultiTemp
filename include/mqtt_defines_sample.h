/**
 * Definitions of MQTT server and credentials
 * (Copy to mqtt_defines.h to use)
 */

#define MQTTSERVER      "mymqttserver.com"
#define MQTTPORT        1883

#define MQTTUSER        "mqttuser"
#define MQTTPASS        "mqttpassword"

#define TOPIC_TELE      "multitemp/%C/tele/"
#define TOPIC_STATUS    "multitemp/%C/status/"
#define TOPIC_CMD       "multitemp/%C/cmd/"
