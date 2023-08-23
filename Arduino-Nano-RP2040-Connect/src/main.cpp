#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include <WiFiNINA.h>
#include "sensorDataHandler.h"
#include "systemConfig.h"
#include "mqttHandler.h"
// #include "cmsis_os2.h"

/*=== Declarations ===========================================================*/
Adafruit_BME680 bme; // I2C
SensorDataHandler sensorHandler(influxdb_host, influxdb_port, influxdb_org, influxdb_bucket, influxdb_token);
EnvironmentalData envData;
int sequenceNumber = 0;
MQTTClient mqtt;

// osThreadId_t threadHandle;
//  const osThreadAttr_t threadAttr1 = {
//      .name = "Thread_SendSensorDataToTheCloud",
//      .stack_size = 1024};

// const osThreadAttr_t threadAttr2 = {
//     .name = "Thread_PrintOutThreads",
//     .stack_size = 4096};

/*=== Private Function Prototypes ============================================*/
static void mySerialSender(const String &message);
static void messageReceived(String &topic, String &payload);
static void errorMode(String error_message);
static void errorMode(void);
static void PresentSensorDataOnSerialInterace(EnvironmentalData envData);
static void IndicateSuccessfulSetupPhase(void);

#include "mbed.h"
#include "mbed_mem_trace.h"

void print_memory_info()
{
  SERIAL.println("");
  char str[80];
  // allocate enough room for every thread's stack statistics
  int cnt = osThreadGetCount();
  mbed_stats_stack_t *stats = (mbed_stats_stack_t *)malloc(cnt * sizeof(mbed_stats_stack_t));

  cnt = mbed_stats_stack_get_each(stats, cnt);
  for (int i = 0; i < cnt; i++)
  {
    snprintf(str, sizeof(str), "Thread: %s, Stack size: %lu / %lu",
             osThreadGetName((void *)stats[i].thread_id), stats[i].max_size, stats[i].reserved_size);
    SERIAL.println(str);
  }
  free(stats);

  // Grab the heap statistics
  mbed_stats_heap_t heap_stats;
  mbed_stats_heap_get(&heap_stats);
  snprintf(str, sizeof(str), "Heap size: %lu / %lu bytes", heap_stats.current_size,
           heap_stats.reserved_size);
  SERIAL.println(str);
  SERIAL.println("");
}

void Thread_SendSensorDataToTheCloud(void *arg)
{
  while (1)
  {
    EnvironmentalData envData = sensorHandler.collectSensorValues();

    if (sensorHandler.sendToCloud(envData) < 0)
      errorMode("Failed to send data to the cloud.");

    if (SERIAL)
      PresentSensorDataOnSerialInterace(envData);

    osDelay(10000);
  }
}

void Thread_PrintOutThreads(void *arg)
{
  while (1)
  {
    print_memory_info();
    osDelay(2000);
  }
}

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
    SERIAL.print("WiFi ...");
  }

  SERIAL.println("Connected to WiFi. IP: ");
  SERIAL.println(WiFi.localIP());
  SERIAL.println("Trying to connect to target host ...");

  if (!sensorHandler.initSensors())
    errorMode("Failed to initialize sensors.");

  mqtt_setup(mqtt, mqtt_broker, mqtt_port, mqtt_username, mqtt_password, messageReceived, mySerialSender);
  mqtt_subscribe(mqtt, "test/topic");
  mqtt_subscribe(mqtt, "node/led1");

  // threadHandle = osThreadNew(Thread_SendSensorDataToTheCloud, NULL, &threadAttr1);
  // threadHandle = osThreadNew(Thread_PrintOutThreads, NULL, &threadAttr2);

  IndicateSuccessfulSetupPhase();
}

/*-----------------------------------------------------------------*/
void loop()
{
  static unsigned long previousMillisLed = 0;
  const unsigned long intervalLed = 800;
  static unsigned long previousMillisDataCollection = 0;
  const unsigned long intervalDataCollection = 10000;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisLed >= intervalLed)
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    previousMillisLed = currentMillis;
  }

  if (currentMillis - previousMillisDataCollection >= intervalDataCollection)
  {
    EnvironmentalData envData = sensorHandler.collectSensorValues();

    if (sensorHandler.sendToCloud(envData) < 0)
      errorMode("Failed to send data to the cloud.");

    if (SERIAL)
      PresentSensorDataOnSerialInterace(envData);
    previousMillisDataCollection = currentMillis;
  }

  mqtt_loop(mqtt);
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