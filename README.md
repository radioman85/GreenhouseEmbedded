# BalkonWinterGarten

## About this repo
This repo contains firmware the arduino plattform. It shall primerally controll a stepper motor using a X-NUCLEO-IHM03A1 shield depending on temperature, that is read from a BME680.
To gain some over engineering it will also connect to a local MQTT broker throuh which the motor controlling can be influenced (i.e. Open, Close, Auto). It also sends environmental data to a local InfluxDB.

## Content
This repo contains two hardware specific projects:
 - first: Arduino NANO RP2040 Connect. This ironically doesn't implement the stepper motor shield as it doesn't have that Arduion default interface. However, the purpose was simply to get know and test that board and the technologies of InfluxDB, MQTT and Docker.
 - second: Arduino UNO WIFI. This contains the entire system. I.e. sensor controlling, stepper motor controlling through MQTT subscribtion and tranfer of data to InfluxDB.

## Hardware
 - Arduino UNO WIFI (Rev2) (or Arduino NANO RP2040)
 - X-NUCLEO-IHM03A1 Stepper Motor Shield (not very suitable for the NANO interface)
 - BME688 Air Quality Sensor Shield (pimoroni.com)
 
## Faced Issues - Arduino UNO vs. Arduino UNO WIFI
In contrary to the Arduino UNO (and Arduino NANO) the interfaces on the Arduino UNO WIFI are slightly different in respect to I2C and SPI. Using the SPI one has to use the ICSP interface and for I2C one has to use other ports.
### SPI
Use SCK, MOSI and MISO of the ICSP interface instead of D10 ... D13
![image](https://user-images.githubusercontent.com/25708993/229466985-0011e3c3-9bf6-434c-942e-5006addbeef8.png)


### I2C
Instead of using A4 (i.e. D18) and A5 (i.e. D19) use D20 for SDA and D21 for SCL.
![image](https://user-images.githubusercontent.com/25708993/229468038-f914c920-033d-463b-8e65-342af5bd1806.png)
