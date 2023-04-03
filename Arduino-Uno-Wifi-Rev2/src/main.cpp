#include <Arduino.h>
#include "mqttHandler.h"
#include "sensorDataHandler.h"
#include "StepperHandler.h"
#include "systemConfig.h"
#include <WiFiNINA.h>

enum WINDOW_STATE_t
{
  WIN_CLOSED,
  WIN_OPEN
};

enum WINDOW_CTRL_MODE_t
{
  MANUAL,
  AUTO
};

/*=== Declarations ===========================================================*/
Adafruit_BME680 bme; // I2C
SensorDataHandler sensorHandler(influxdb_host, influxdb_port, influxdb_org, influxdb_bucket, influxdb_token);
EnvironmentalData envData;
int sequenceNumber = 0;

StepperHandler stepper(nCS_PIN, STCK_PIN, nSTBY_nRESET_PIN, nBUSY_PIN);

String command = "";
MQTTClient mqtt;

WINDOW_STATE_t windowState = WIN_CLOSED;
WINDOW_CTRL_MODE_t windowCtrlMode = AUTO;

/*=== Private Function Prototypes ============================================*/
static void welcomeScreen(void);
static void IndicateSuccessfulSetupPhase(void);
static void mySerialSender(const String &message);
static void messageReceived(String &topic, String &payload);
static void errorMode(String error_message);
static void errorMode(void);
static void manualMode(String command);
static void autoMode(float temperature);
static void collectData(float *temperature);
static void PresentSensorDataOnSerialInterace(EnvironmentalData envData);

/*=== Public Functions =======================================================*/
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  SERIAL.begin(115200);
  welcomeScreen();

  if (!sensorHandler.initSensors())
    errorMode("Failed to initialize sensors.");

  if (WiFi.status() == WL_NO_MODULE)
    errorMode("Communication with WiFi module failed!");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    SERIAL.print("WiFi ...");
  }

  SERIAL.println("Connected to WiFi. IP: ");
  SERIAL.println("Trying to connect to target host ...");
  SERIAL.println(WiFi.localIP());

  mqtt_setup(mqtt, mqtt_broker, mqtt_port, mqtt_username, mqtt_password, messageReceived, mySerialSender);
  mqtt_subscribe(mqtt, "balconygarden/greenhouse/window");

  stepper.init();

  IndicateSuccessfulSetupPhase();
}

void loop()
{
  unsigned long currentMillis = millis();
  static unsigned long previousMillisDataCollection = 0;
  const unsigned long intervalDataCollection = 5000;
  static float temperature = 20.0;

  if (currentMillis - previousMillisDataCollection >= intervalDataCollection)
  {
    collectData(&temperature);
    previousMillisDataCollection = currentMillis;
  }

  if (command == "auto")
    windowCtrlMode = AUTO;
  else if (command == "close" || command == "open")
    windowCtrlMode = MANUAL;
  else
    ;

  stepper.init(); // Required, as mqtt_loop somehow deinit the stepper. To be investigated.

  switch (windowCtrlMode)
  {
  case MANUAL:
    manualMode(command);
    command = "";
    break;

  case AUTO:
    autoMode(temperature);
    break;

  default:
    break;
  }

  mqtt_loop(mqtt);
}

/*=== Private Functions ======================================================*/
static void welcomeScreen(void)
{
  SERIAL.println("--- Stepper Motor: Welcome --------------------");
}

/*-----------------------------------------------------------------*/
static void IndicateSuccessfulSetupPhase(void)
{
  if (SERIAL)
    SERIAL.println("--- Initialization successfull! Have fun. -----");

  for (size_t i = 0; i < 3; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(50);
    digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
    delay(100);
  }
}

/*-----------------------------------------------------------------*/
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
  command = payload;
  command.trim();
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
static void manualMode(String command)
{
  if (command == "open" && windowState != WIN_OPEN)
  {
    for (size_t i = 0; i < 8; i++)
    {
      delay(150);
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    stepper.move(StepperHandler::FORWARD, 100000);
    windowState = WIN_OPEN;
  }

  if (command == "close" && windowState != WIN_CLOSED)
  {
    for (size_t i = 0; i < 4; i++)
    {
      delay(300);
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    stepper.move(StepperHandler::BACKWARD, 100000);
    windowState = WIN_CLOSED;
  }
}

/*-----------------------------------------------------------------*/
static void autoMode(float temperature)
{
  if (temperature > 28.0 && windowState == WIN_CLOSED)
  {
    stepper.move(StepperHandler::FORWARD, 200000);
    windowState = WIN_OPEN;
    SERIAL.print("Temperature raised above 27°C");
  }

  if (temperature < 27.0 && windowState == WIN_OPEN)
  {
    stepper.move(StepperHandler::BACKWARD, 200000);
    windowState = WIN_CLOSED;
    SERIAL.print("Temperature fell below 26°C ");
  }
}

/*-----------------------------------------------------------------*/
static void collectData(float *tempeature_p)
{
  EnvironmentalData envData = sensorHandler.collectSensorValues();

  if (sensorHandler.sendToCloud(envData) < 0)
    errorMode("Failed to send data to the cloud.");

  if (SERIAL)
    PresentSensorDataOnSerialInterace(envData);

  *tempeature_p = envData.temperature;
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