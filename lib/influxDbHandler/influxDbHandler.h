#ifndef INFLUXDBHANDLER_H
#define INFLUXDBHANDLER_H

#include <WiFiNINA.h>
#include <Arduino.h> // include this if you want to use the STL in your library

class InfluxDbHandler
{
public:
    InfluxDbHandler(const char *host, uint16_t port, const char *org, const char *bucket, const char *token);
    int16_t sendData(const char *measurement_name, const char *type, float value);

private:
    const char *_host;
    uint16_t _port;
    const char *_org;
    const char *_bucket;
    const char *_token;
    WiFiClient _client;
};

#endif // INFLUXDBHANDLER_H