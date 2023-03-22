#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "influxDbHandler.h"
#include "EnvironmentalData.h"

class SensorDataHandler
{
public:
    SensorDataHandler(const char *host, uint16_t port, const char *org, const char *bucket, const char *token);
    bool initSensors();
    EnvironmentalData collectSensorValues();
    int16_t sendToCloud(EnvironmentalData);

private:
    Adafruit_BME680 bme;
    InfluxDbHandler dbHandler;

    float getTemperature();
    float getHumidity();
    float getPressure();
};