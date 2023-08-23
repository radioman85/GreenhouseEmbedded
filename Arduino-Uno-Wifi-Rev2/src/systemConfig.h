#define nCS_PIN 10
#define STCK_PIN 9
#define nSTBY_nRESET_PIN 8
#define nBUSY_PIN 4

#define SERIAL Serial1
#define SERIAL_BAUDRATE 115200

const char *ssid = "{your_ssid}";
const char *password = "{your_password}";

#define HOST "localhost"

const char *mqtt_broker = "192.168.1.25";
int mqtt_port = 1883;
const char *mqtt_username = "your_mqtt_username";
const char *mqtt_password = "your_mqtt_password";

const char *influxdb_host = "192.168.1.25";
const uint16_t influxdb_port = 8086;
const char *influxdb_org = "huntabyte";
const char *influxdb_bucket = "MyBucket";
const char *influxdb_token = "7QOfRe-ipnNtnXoFPtIdR14u92YikLxHeTiv2coZxPYqc5scTAnu-oq_HQYDf3FOohnlSWgjZyHhTLJYcTKSXA==";