#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <WiFiNINA.h>
#include <MQTT.h>

// typedef std::function<void(String &, String &)> MQTTClientCallbackSimpleFunction;
typedef void (*MessageRecevied)(const String &topic, const String &payload);
typedef void (*MessageSender)(const String &message);

void mqtt_setup(MQTTClient &mqtt, const char *mqtt_broker, int mqtt_port, const char *mqtt_username, const char *mqtt_password, MessageRecevied messageReceived, MessageSender messageSender);
void mqtt_loop(MQTTClient &mqtt);
void mqtt_subscribe(MQTTClient &mqtt, const String &mqtt_topic);
void mqtt_publish_system_status(MQTTClient &mqtt, const String &status, const String &mode);

#endif // MQTT_HANDLER_H
