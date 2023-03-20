#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme; // I2C

#define SERIAL_BAUDRATE 115200
#define PACKET_START "TransferingBmeData"
#define PACKET_END "EndBmeData"

int32_t loopCount = 0;

int keyIndex = 0;

char server[] = "www.google.com";

int status = WL_IDLE_STATUS;

const char *ssid = "NETGEAR16";
const char *password = "friendlycanoe810";

const char *influxdb_host = "192.168.1.22";
const uint16_t influxdb_port = 8086;
const char *influxdb_org = "huntabyte";
// const char *influxdb_org = "MyInflux";
const char *influxdb_bucket = "MyBucket";
const char *influxdb_token = "0V_m85vb5WOviAJIDPjViPsA1Loa2UwnLYub6g_oXqmDUg6V3Hw_KE7vZ4B3LJvvOc9py6KrTEGuP8eHgVs2Ig==";
// const char *influxdb_token = "qCiIZGeECXinMEhQ3iYE1WU9qjxEDlmxQgv0qCBcqQqdiNR2HdUpW9NWxZLcX4xyEgNBLG6KHEdKR2BZY9Qr4g==";

IPAddress remoteIP_NiksPc(192, 168, 1, 22);

WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, influxdb_host, influxdb_port);

String hostName = "www.google.com";
int pingResult;

int sequenceNumber = 0;

// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

static void sendDataToInfluxDB(String measurement_name, String type, float value);
static void ping(IPAddress ipAddress);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial1.begin(SERIAL_BAUDRATE);
  while (!Serial1)
  {
    ;
  }

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial1.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial1.print(".");
  }

  Serial1.println("Connected to WiFi. IP: ");
  Serial1.println(WiFi.localIP());

  Serial1.println("Trying to connect to target host ...");

  if (!bme.begin(0x76, 1))
  {
    Serial1.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1)
      ;
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // influx.setBucket(INFLUXDB_BUCKET);
  // influx.setVersion(2);
  // influx.setOrg(INFLUXDB_ORG);
  // influx.setPort(8086);
  // influx.setToken(INFLUXDB_TOKEN);
  Serial1.println("HelloFromNode"); // send a capital A

  for (size_t i = 0; i < 3; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(50);
    digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
    delay(100);
  }
}

void loop()
{
  sequenceNumber++;

  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  if (Serial1.available() > 0)
  {
    Serial1.println("TransferingBmeData:");
    Serial1.print("temperature:");
    Serial1.print(temperature);
    Serial1.print(" humidity:");
    Serial1.print(humidity);
    Serial1.print(" pressure:");
    Serial1.println(pressure);
    Serial1.println("EndBmeData:");
  }

  Serial1.print(sequenceNumber);
  Serial1.print(" - Transfer data ");
  Serial1.println("---------------------------------------");
  sendDataToInfluxDB("sensor_measurement", "temperature", temperature);
  sendDataToInfluxDB("sensor_measurement", "humidity", humidity);
  sendDataToInfluxDB("sensor_measurement", "pressure", pressure);
  Serial1.println("Data transfered");
  delay(2000);
}

static void sendDataToInfluxDB(String measurement_name, String type, float value)
{
  WiFiClient client;

  if (!client.connect(influxdb_host, influxdb_port))
  {
    Serial1.println("Connection to InfluxDB failed");
    return;
  }

  Serial1.println("Connected to InfluxDb");

  String lineProtocol = measurement_name + " " + type + "=" + String(value);

  client.print("POST /api/v2/write?org=");
  client.print(influxdb_org);
  client.print("&bucket=");
  client.print(influxdb_bucket);
  client.print(" HTTP/1.1\r\n");
  client.print("Host: ");
  client.print(influxdb_host);
  client.print(":");
  client.print(influxdb_port);
  client.print("\r\n");
  client.print("Authorization: Token ");
  client.print(influxdb_token);
  client.print("\r\n");
  client.print("Content-Type: text/plain\r\n");
  client.print("Content-Length: ");
  client.print(lineProtocol.length());
  client.print("\r\n");
  client.print("\r\n");
  client.print(lineProtocol);

  client.stop();
}

static void ping(IPAddress ipAddress)
{
  int rtt = WiFi.ping(ipAddress);
  if (rtt >= 0)
  {
    Serial1.print("Ping to ");
    Serial1.print(ipAddress);
    Serial1.print(" successful. RTT: ");
    Serial1.print(rtt);
    Serial1.println(" ms");
  }
  else
  {
    Serial1.print("Ping to ");
    Serial1.print(ipAddress);
    Serial1.println(" failed.");
  }
}