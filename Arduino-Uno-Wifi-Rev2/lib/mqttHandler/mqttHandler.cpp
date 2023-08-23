#include "mqttHandler.h"

WiFiClient wifiClient;

void connectMQTT(MQTTClient &mqtt, const char *mqtt_broker, int mqtt_port, const char *mqtt_username, const char *mqtt_password, MessageSender messageSender)
{
    if (messageSender)
        messageSender("Connecting to MQTT...");

    mqtt.begin(mqtt_broker, mqtt_port, wifiClient);
    mqtt.connect(mqtt_username, mqtt_password);

    while (!mqtt.connected())
    {
        delay(1000);
        if (messageSender)
            messageSender(".");
    }

    if (messageSender)
        messageSender(" connected.");
}

void mqtt_setup(MQTTClient &mqtt, const char *mqtt_broker, int mqtt_port, const char *mqtt_username, const char *mqtt_password, MessageRecevied messageReceived, MessageSender messageSender)
{
    connectMQTT(mqtt, mqtt_broker, mqtt_port, mqtt_username, mqtt_password, messageSender);
    mqtt.onMessage(messageReceived);
}

void mqtt_loop(MQTTClient &mqtt)
{
    mqtt.loop();
}

void mqtt_subscribe(MQTTClient &mqtt, const String &mqtt_topic)
{
    mqtt.subscribe(mqtt_topic);
}