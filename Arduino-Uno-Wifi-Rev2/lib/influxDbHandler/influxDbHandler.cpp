#include "influxDbHandler.h"

InfluxDbHandler::InfluxDbHandler(const char *host, uint16_t port, const char *org, const char *bucket, const char *token)
    : _host(host), _port(port), _org(org), _bucket(bucket), _token(token)
{
}

int16_t InfluxDbHandler::sendToCloud(EnvironmentalData data)
{
    sendData("sensor_measurement", "temperature", data.temperature);
    sendData("sensor_measurement", "humidity", data.humidity);
    sendData("sensor_measurement", "pressure", data.pressure);
    return 0;
}

int16_t InfluxDbHandler::sendData(const char *measurement_name, const char *type, float value)
{
    if (!_client.connect(_host, _port))
        return -1;

    String lineProtocol = String(measurement_name) + " " + String(type) + "=" + String(value);

    _client.print("POST /api/v2/write?org=");
    _client.print(_org);
    _client.print("&bucket=");
    _client.print(_bucket);
    _client.print(" HTTP/1.1\r\n");
    _client.print("Host: ");
    _client.print(_host);
    _client.print(":");
    _client.print(_port);
    _client.print("\r\n");
    _client.print("Authorization: Token ");
    _client.print(_token);
    _client.print("\r\n");
    _client.print("Content-Type: text/plain\r\n");
    _client.print("Content-Length: ");
    _client.print(lineProtocol.length());
    _client.print("\r\n");
    _client.print("\r\n");
    _client.print(lineProtocol);

    _client.stop();

    return 0;
}