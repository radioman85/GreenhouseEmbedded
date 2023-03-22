const char *ssid = "NETGEAR16";
const char *password = "friendlycanoe810";

// const char *ssid = "MAMMUT";
// const char *password = "cc$3+ckl.7.-RV";

const char *influxdb_host = "192.168.1.22";
const uint16_t influxdb_port = 8086;
const char *influxdb_org = "huntabyte";
const char *influxdb_bucket = "MyBucket";
const char *influxdb_token = "0V_m85vb5WOviAJIDPjViPsA1Loa2UwnLYub6g_oXqmDUg6V3Hw_KE7vZ4B3LJvvOc9py6KrTEGuP8eHgVs2Ig==";

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SERIAL Serial1
#define SERIAL_BAUDRATE 115200