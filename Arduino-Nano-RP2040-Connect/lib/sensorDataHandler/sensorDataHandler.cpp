#include "sensorDataHandler.h"
#include "influxDbHandler.h"

// #define SIMULATE_SENSOR

SensorDataHandler::SensorDataHandler(const char *host, uint16_t port, const char *org, const char *bucket, const char *token)
    : dbHandler(host, port, org, bucket, token)
{
}

bool SensorDataHandler::initSensors()
{
#ifndef SIMULATE_SENSOR
    // Initialize the BME680 sensor
    if (!bme.begin(0x76, 1))
        return false;

    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms
#endif
    return true;
}

EnvironmentalData SensorDataHandler::collectSensorValues()
{
    EnvironmentalData data;

#ifndef SIMULATE_SENSOR
    data.temperature = bme.readTemperature();
    data.humidity = bme.readHumidity();
    data.pressure = bme.readPressure() / 100.0F;
#else
    data.temperature = 20.0;
    data.humidity = 50.0;
    data.pressure = 1013.25;
#endif
    return data;
}

int16_t SensorDataHandler::sendToCloud(EnvironmentalData data)
{
    if (dbHandler.sendToCloud(data) < 0)
        return -1;

    return 0;
}
