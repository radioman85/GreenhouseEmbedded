#include "StepperHandler.h"

StepperHandler::StepperHandler(uint8_t nCS_PIN, uint8_t STCK_PIN, uint8_t nSTBY_nRESET_PIN, uint8_t nBUSY_PIN) : driver(0, nCS_PIN, nSTBY_nRESET_PIN),
                                                                                                                 _nCS_PIN(nCS_PIN),
                                                                                                                 _STCK_PIN(STCK_PIN),
                                                                                                                 _nSTBY_nRESET_PIN(nSTBY_nRESET_PIN),
                                                                                                                 _nBUSY_PIN(nBUSY_PIN)
{
}

void StepperHandler::init()
{
    pinMode(_nSTBY_nRESET_PIN, OUTPUT);
    pinMode(_nCS_PIN, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(MISO, OUTPUT);
    pinMode(SCK, OUTPUT);

    // Reset powerSTEP and set CS
    digitalWrite(_nSTBY_nRESET_PIN, HIGH);
    digitalWrite(_nSTBY_nRESET_PIN, LOW);
    digitalWrite(_nSTBY_nRESET_PIN, HIGH);
    digitalWrite(_nCS_PIN, HIGH);

    // Start SPI
    SPI.begin();
    SPI.setDataMode(SPI_MODE3);

    // Configure powerSTEP
    driver.SPIPortConnect(&SPI);
    driver.configSyncPin(_nBUSY_PIN, 0);
    driver.configStepMode(STEP_FS_128);

    driver.setMaxSpeed(1000);
    driver.setFullSpeed(2000);
    driver.setAcc(2000);
    driver.setDec(2000);

    driver.setSlewRate(SR_520V_us);

    driver.setOCThreshold(8);
    driver.setOCShutdown(OC_SD_ENABLE);

    driver.setPWMFreq(PWM_DIV_1, PWM_MUL_0_75);

    driver.setVoltageComp(VS_COMP_DISABLE);

    driver.setSwitchMode(SW_USER);

    driver.setOscMode(INT_16MHZ);

    driver.setRunKVAL(64);
    driver.setAccKVAL(64);
    driver.setDecKVAL(64);
    driver.setHoldKVAL(8);

    driver.setParam(ALARM_EN, 0x8F);

    driver.getStatus();
}

void StepperHandler::nSteps(bool direction, uint32_t steps)
{
    // Move the motor a specified number of steps in the given direction
    if (direction == FORWARD)
        driver.move(FWD, steps);
    else
        driver.move(REV, steps);

    while (driver.busyCheck())
        ;
    driver.softStop();
}

void StepperHandler::move(bool direction, uint32_t steps)
{
    // Run the motor continuously in the given direction
    if (direction == FORWARD)
        driver.move(FWD, steps);
    else
        driver.move(REV, steps);

    while (driver.busyCheck())
        ;
}



void StepperHandler::stop()
{
    driver.softStop();
    while (driver.busyCheck())
        ;
}