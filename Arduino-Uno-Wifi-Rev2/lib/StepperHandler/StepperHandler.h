#include <Ponoor_PowerSTEP01Library.h>

class StepperHandler
{
public:
    enum Direction
    {
        FORWARD = FWD,
        BACKWARD = REV
    };

    StepperHandler(uint8_t nCS_PIN, uint8_t STCK_PIN, uint8_t nSTBY_nRESET_PIN, uint8_t nBUSY_PIN);
    void init();
    void nSteps(bool direction, uint32_t steps);
    void move(bool direction, uint32_t steps);
    void stop();

private:
    powerSTEP driver;
    uint8_t _nCS_PIN;
    uint8_t _STCK_PIN;
    uint8_t _nSTBY_nRESET_PIN;
    uint8_t _nBUSY_PIN;
};
