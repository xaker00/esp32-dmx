#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include "cie1931.h"

#define PWM_FREQ 300 // Analog servos run at ~50 Hz updates

enum COLORS{
    RED,
    GREEN,
    BLUE,
    WHITE
};

class PwmControl
{

private:
    Adafruit_PWMServoDriver *pwm;
    uint16_t frequency;

public:
    PwmControl(uint8_t address = 0x40, float frequency = 500.0);
    ~PwmControl();
    void begin();
    void SetColor(uint8_t channel, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);
    void SetRed(uint8_t channel, uint8_t value);
    void SetGreen(uint8_t channel, uint8_t value);
    void SetBlue(uint8_t channel, uint8_t value);
    void SetWhite(uint8_t channel, uint8_t value);
    void On(uint8_t channel);
    void Off(uint8_t channel);
};

class ColorData{
    public:
        ColorData(COLORS channel, uint16_t value);
        COLORS channel;
        uint16_t value;
};