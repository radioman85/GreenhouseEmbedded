#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "influxDbHandler.h"
#include "systemConfig.h"

// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme; // I2C
int sequenceNumber = 0;
InfluxDbHandler dbHandler(influxdb_host, influxdb_port, influxdb_org, influxdb_bucket, influxdb_token);

static void error_mode(void);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial1.begin(SERIAL_BAUDRATE);
  while (!Serial1)
    ;

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial1.println("Communication with WiFi module failed!");
    error_mode();
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
    error_mode();
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

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

  dbHandler.sendData("sensor_measurement", "temperature", temperature);
  dbHandler.sendData("sensor_measurement", "humidity", humidity);
  dbHandler.sendData("sensor_measurement", "pressure", pressure);
  Serial1.println("Data transfered");
  delay(10000);
}

static void error_mode(void)
{
  while (true)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
  }
}