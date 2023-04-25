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

bool wifiIsAvailable = false;

/*=== Private Function Prototypes ============================================*/
static void lifeSign(void);
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
static void toggle(int pin);
static void loopMqtt(void);

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

  uint16_t countWifi = 0;
  while (true)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      wifiIsAvailable = true;
      SERIAL.println("Connected to WiFi. IP: ");
      SERIAL.println("Trying to connect to target host ...");
      SERIAL.println(WiFi.localIP());
      break;
    }

    if (countWifi++ == 5)
    {
      wifiIsAvailable = false;
      SERIAL.println("WiFi not connected");
      break;
    }

    delay(500);
    SERIAL.print("WiFi ...");
  }

  if (wifiIsAvailable)
  {
    mqtt_setup(mqtt, mqtt_broker, mqtt_port, mqtt_username, mqtt_password, messageReceived, mySerialSender);
    mqtt_subscribe(mqtt, "balconygarden/greenhouse/window");
  }

  stepper.init();

  IndicateSuccessfulSetupPhase();
}

void loop()
{
  static float temperature = 20.0;

  lifeSign();
  collectData(&temperature);

  if (command == "auto")
    windowCtrlMode = AUTO;
  else if (command == "close" || command == "open")
    windowCtrlMode = MANUAL;
  else
    ;

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

  if (wifiIsAvailable)
    loopMqtt();
}

/*=== Private Functions ======================================================*/
static void lifeSign(void)
{
  unsigned long currentMillis = millis();
  static unsigned long previousMillisDataCollection = currentMillis;
  const unsigned long intervalDataCollection = 2000;

  if (currentMillis - previousMillisDataCollection >= intervalDataCollection)
  {
    previousMillisDataCollection = currentMillis;
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(50);
    digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
    SERIAL.println("LED: I'm alive.");
  }

  static unsigned long previousMilliMqttStatus = currentMillis;
  const unsigned long intervalMqttStatus = 5000;

  if (currentMillis - previousMilliMqttStatus >= intervalMqttStatus)
  {
    previousMilliMqttStatus = currentMillis;

    String _windowState = windowState == WIN_CLOSED ? "closed" : "open";
    String _windowCtrlMode = windowCtrlMode == MANUAL ? "manual" : "auto";

    mqtt_publish_system_status(mqtt, _windowState, _windowCtrlMode);
    SERIAL.println("MQTT: I'm alive.");
  }
}

/*-----------------------------------------------------------------*/
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
  if (command == "open" && windowState == WIN_CLOSED)
  {
    windowState = WIN_OPEN;
    SERIAL.println("*** Opening window *****************************************");
    stepper.init(); // Required, as mqtt_loop somehow deinit the stepper. To be investigated.
    stepper.move(StepperHandler::FORWARD, 100000);

    mqtt_publish_system_status(mqtt, "open", "manual");
  }

  if (command == "close" && windowState == WIN_OPEN)
  {
    windowState = WIN_CLOSED;
    SERIAL.println("*** Closing window *****************************************");
    stepper.init(); // Required, as mqtt_loop somehow deinit the stepper. To be investigated.
    stepper.move(StepperHandler::BACKWARD, 100000);

    mqtt_publish_system_status(mqtt, "closed", "manual");
  }

  if (command == "singleopen")
  {
    windowState = WIN_OPEN;
    SERIAL.println("*** Single opening step ************************************");
    stepper.init(); // Required, as mqtt_loop somehow deinit the stepper. To be investigated.
    stepper.move(StepperHandler::FORWARD, 20000);
  }

  if (command == "singleclose" && windowState == WIN_OPEN)
  {
    SERIAL.println("*** Single closing step ************************************");
    stepper.init(); // Required, as mqtt_loop somehow deinit the stepper. To be investigated.
    stepper.move(StepperHandler::BACKWARD, 20000);
  }

  if (command == "setclose")
  {
    windowState = WIN_CLOSED;
    SERIAL.println("*** Determined closed window********************************");
    stepper.init(); // Required, as mqtt_loop somehow deinit the stepper. To be investigated.

    mqtt_publish_system_status(mqtt, "closed", "manual");
  }
}

/*-----------------------------------------------------------------*/
static void autoMode(float temperature)
{
  const float upperTempLimit = 31.0;
  const float lowerTempLimit = 30.0;

  if (temperature > upperTempLimit && windowState == WIN_CLOSED)
  {
    stepper.init(); // Required, as mqtt_loop somehow deinit the stepper. To be investigated.
    stepper.move(StepperHandler::FORWARD, 200000);
    windowState = WIN_OPEN;
    SERIAL.print("*** Temperature raised above");
    SERIAL.print(upperTempLimit);
    SERIAL.println("°C *************************************");

    mqtt_publish_system_status(mqtt, "open", "auto");
    return;
  }

  if (temperature < lowerTempLimit && windowState == WIN_OPEN)
  {
    stepper.init(); // Required, as mqtt_loop somehow deinit the stepper. To be investigated.
    stepper.move(StepperHandler::BACKWARD, 200000);
    windowState = WIN_CLOSED;
    SERIAL.print("*** Temperature fell below ");
    SERIAL.print(lowerTempLimit);
    SERIAL.println("°C *************************************");

    mqtt_publish_system_status(mqtt, "closed", "auto");
    return;
  }
}

/*-----------------------------------------------------------------*/
static void collectData(float *temperature_p)
{
  unsigned long currentMillis = millis();
  static unsigned long previousMillisDataCollection = currentMillis;
  const unsigned long intervalDataCollection = 5000;

  static unsigned long previousMillisDataPublishing = currentMillis;
  const unsigned long intervalDataPublishing = 5000;

  if (currentMillis - previousMillisDataCollection >= intervalDataCollection)
  {
    previousMillisDataCollection = currentMillis;
    EnvironmentalData envData = sensorHandler.collectSensorValues();

    if (SERIAL)
      PresentSensorDataOnSerialInterace(envData);

    *temperature_p = envData.temperature;
  }

  if (wifiIsAvailable)
  {
    if (currentMillis - previousMillisDataPublishing >= intervalDataPublishing)
    {
      previousMillisDataPublishing = currentMillis;
      EnvironmentalData envData = sensorHandler.collectSensorValues();

      if (sensorHandler.sendToCloud(envData) < 0)
      {
        if (SERIAL)
          SERIAL.println("Failed to send data to the cloud");
        // errorMode("Failed to send data to the cloud.");
      }
    }
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
static void toggle(int pin)
{
  digitalWrite(pin, !digitalRead(pin));
}

/*-----------------------------------------------------------------*/
static void loopMqtt(void)
{
  mqtt_loop(mqtt);
}