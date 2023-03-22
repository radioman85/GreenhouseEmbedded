#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include <WiFiNINA.h>
#include "sensorDataHandler.h"
#include "systemConfig.h"
#include "mqttHandler.h"

/*=== Declarations ===========================================================*/
Adafruit_BME680 bme; // I2C
SensorDataHandler sensorHandler(influxdb_host, influxdb_port, influxdb_org, influxdb_bucket, influxdb_token);
EnvironmentalData envData;
int sequenceNumber = 0;
MQTTClient mqtt;

/*=== Private Function Prototypes ============================================*/
static void mySerialSender(const String &message);
static void errorMode(String error_message);
static void errorMode(void);
static void PresentSensorDataOnSerialInterace(EnvironmentalData envData);
static void IndicateSuccessfulSetupPhase(void);
static void messageReceived(String &topic, String &payload);
static void mySerialSender(const String &message);

/*=== Public Functions =======================================================*/
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  SERIAL.begin(SERIAL_BAUDRATE);
  while (!SERIAL)
    ;

  if (WiFi.status() == WL_NO_MODULE)
    errorMode("Communication with WiFi module failed!");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    SERIAL.print(".");
  }

  SERIAL.println("Connected to WiFi. IP: ");
  SERIAL.println(WiFi.localIP());
  SERIAL.println("Trying to connect to target host ...");

  if (!sensorHandler.initSensors())
    errorMode("Failed to initialize sensors.");

  mqtt_setup(mqtt, mqtt_broker, mqtt_port, mqtt_username, mqtt_password, messageReceived, mySerialSender);
  mqtt_subscribe(mqtt, "test/topic");

  IndicateSuccessfulSetupPhase();
}

/*-----------------------------------------------------------------*/
void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);

  EnvironmentalData envData = sensorHandler.collectSensorValues();

  if (sensorHandler.sendToCloud(envData) < 0)
    errorMode("Failed to send data to the cloud.");

  if (SERIAL)
    PresentSensorDataOnSerialInterace(envData);

  mqtt_loop(mqtt);

  delay(10000);
}

/*=== Private Functions ======================================================*/
static void mySerialSender(const String &message)
{
  if (SERIAL)
    SERIAL.println(message);
}

/*-----------------------------------------------------------------*/
static void messageReceived(String &topic, String &payload)
{
  SERIAL.print("Message received in topic: ");
  SERIAL.println(topic);

  SERIAL.print("Payload: ");
  SERIAL.println(payload);
}

/*-----------------------------------------------------------------*/
static void errorMode(String error_message)
{
  if (SERIAL)
    SERIAL.print(error_message);

  errorMode();
}

/*-----------------------------------------------------------------*/
static void errorMode(void)
{
  while (true)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
  }
}

/*-----------------------------------------------------------------*/
static void PresentSensorDataOnSerialInterace(EnvironmentalData envData)
{
  SERIAL.print("--- ");
  SERIAL.print(sequenceNumber++);
  SERIAL.println(" - Sensor Data ------------------------");
  SERIAL.print("Temperature: ");
  SERIAL.print(envData.temperature);
  SERIAL.println(" °C");

  SERIAL.print("Humidity: ");
  SERIAL.print(envData.humidity);
  SERIAL.println(" %");

  SERIAL.print("Pressure: ");
  SERIAL.print(envData.pressure);
  SERIAL.println(" hPa");
}

/*-----------------------------------------------------------------*/
void IndicateSuccessfulSetupPhase(void)
{
  if (SERIAL)
    SERIAL.println("HelloFromNode"); // send a capital A

  for (size_t i = 0; i < 3; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(50);
    digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
    delay(100);
  }
}