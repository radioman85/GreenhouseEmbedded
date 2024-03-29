#include "mqttHandler.h"
#include <ArduinoJson.h>

WiFiClient wifiClient;

void connectMQTT(MQTTClient &mqtt, const char *mqtt_broker, int mqtt_port, const char *mqtt_username, const char *mqtt_password, MessageSender messageSender)
{
    if (messageSender)
        messageSender("Connecting to MQTT...");

    mqtt.begin(mqtt_broker, mqtt_port, wifiClient);
    mqtt.connect(mqtt_username, mqtt_password);

    int numberOfTries = 0;
    while (!mqtt.connected())
    {
        delay(1000);
        if (messageSender)
            messageSender("mqtt...");

        if (numberOfTries++ > 4)
        {
            messageSender("MQTT not connected.");
            return;
        }
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

void mqtt_publish_system_status(MQTTClient &mqtt, const String &status, const String &mode)
{
    const char *topic = "balconygarden/greenhouse/windowdriver";

    StaticJsonDocument<64> jsonDoc;
    jsonDoc["status"] = status;
    jsonDoc["mode"] = mode;

    String payload;
    serializeJson(jsonDoc, payload);

    mqtt.publish(topic, payload);
}