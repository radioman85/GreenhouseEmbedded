#define HOST "192.168.1.22"

const char *ssid = "your_ssid";
const char *password = "{your_password}";

const char *influxdb_host = "192.168.1.22";
const uint16_t influxdb_port = 8086;
const char *influxdb_org = "huntabyte";
const char *influxdb_bucket = "MyBucket";
const char *influxdb_token = "0V_m85vb5WOviAJIDPjViPsA1Loa2UwnLYub6g_oXqmDUg6V3Hw_KE7vZ4B3LJvvOc9py6KrTEGuP8eHgVs2Ig==";

const char *mqtt_broker = "192.168.1.22";
int mqtt_port = 1883;
const char *mqtt_username = "your_mqtt_username";
const char *mqtt_password = "your_mqtt_password";

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SERIAL Serial1
#define SERIAL_BAUDRATE 115200