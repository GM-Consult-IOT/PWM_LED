/*!
* @file /*!
* @file PWM_LED.h
*
* @mainpage Status LED interface for 400 Ocean Series Devices.
*
* @section intro_sec_Introduction
*
* Status LED interface for 400 Ocean Series Devices with an RGB status LED.
* 
* @section author Author
* 
* Gerhard Malan for GM Consult Pty Ltd
* 
 * @section license License
 * 
 * This library is open-source under the BSD 3-Clause license and 
 * redistribution and use in source and binary forms, with or without 
 * modification, are permitted, provided that the license conditions are met.
 * 
*/



#ifndef __GPIO_LED_H__
#define __GPIO_LED_H__

/// Uncomment to see debugging out put on [Serial].
// #define GPIO_LED_DEBUG

#include <Arduino.h>
#include <iostream>
#include <algorithm>

#define GPIO_LED_PWM_RESOLUTION 8     
#define GPIO_LED_PWM_FREQ 100     

const uint16_t GPIO_LED_PWM_MAX_DUTY_CYCLE = pow(2, GPIO_LED_PWM_RESOLUTION) - 1;

/// @brief Enumeration of LED color as combinations of red, green and blue
/// expressed as 16-bit color values.
typedef enum LED_Color{
    COLOR_RED = 0xf00,
    COLOR_GREEN = 0x0f0,
    COLOR_BLUE = 0x00f,
    COLOR_YELLOW = 0xff0,
    COLOR_MAGENTA = 0xf0f,
    COLOR_CYAN = 0x0ff,
} GPIO_LED_color_t;

/// @brief Enumeration of LED state.
typedef enum LED_State{

    /// @brief The LED is ON.
    LED_OFF = 0x00,

    /// @brief The LED is OFF.
    LED_ON = 0X01,

    /// @brief The LED is FLASHING.
    LED_FLASHING = 0x10,

}led_state_t;

/// @brief Defines the properties of a status LED and exposes 
/// methods to turn the LED on or off.
class PWM_LED{
    
    public:

    PWM_LED(uint8_t pin,
             uint8_t PwmChannel,
             int & brightness, 
             int onState = LOW);

    /// @brief Initializes the LED and then turns it OFF.
    /// @param brightness The brightness of the LED when it is turned on. 
    /// Defaults to 0xFF (100%).
    /// @return true if initialization completed without errors.
    bool begin();

    /// @brief Writes _onState to the GPIO pin and cancels any 
    /// flashing if previously enabled.
    ///
    /// If the brightness parameter is not passed in or 0 then the current
    /// brightness level will be used.
    /// @param brightness The brightness of the LED when it is on. 
    void on();

    /// @brief Writes _offState() to the GPIO pin and cancels any 
    /// flashing if previously enabled.
    void off();

    /// @brief Flashes the LED at a periodic interval of [periodMS] for a duration
    /// of [durationMs]. If [durationMs] is 0 it will be set to  `periodMs / 2`.
    /// If [durationMs] is greater than or equal to [periodMs] then the LED will 
    /// be turned on all the time (no flashing).
    ///
    /// If the brightness parameter is not passed in or 0 then the current
    /// brightness level will be used.
    ///
    /// To stop the flashing of the LED call `off()`.
    /// @param pattern The cycle period for the flashing of the LED in milliseconds.
    /// @param length the length of the [pattern] array.
    /// @param brightness The brightness of the LED when it is on. 
    void flash(uint16_t * pattern, uint8_t length);

    /// @brief The current state of the LED.
    /// @return Returns the current LED state.
    LED_State state();

    protected:

    /// @brief Task handle for LED flashing task.
    TaskHandle_t _flashTask = NULL;

    /// @brief Semaphore handle for starting flasher
    SemaphoreHandle_t _flashSemaphore = NULL;

    /// @brief The LED flashing task.
    /// @param  void.
    void _flash(void);

    private:

    /// @brief Private variable holding the on PWM duty cycle of the
    /// LED PWM channel.
    int & _brightness;
    
    /// @brief The static delegate of [_readSensor]
    /// @param _this NULL
    static void _flashTaskStatic(void* _this);

    /// @brief Private function to create the FreeRTOS semaphore 
    /// and task.
    /// @return true if initialization completed without errors.
    bool _createTask();

    /// @brief The period of the LED flashing cycle.
    uint16_t _flashPattern[255];

    /// @brief The duration of each flash.
    uint8_t _flashPatternLength = 0;
    
    /// @brief The GPIO pin that the LED is attached to.
    uint8_t _GPIO;

    uint8_t _PwmChannel;

    /// @brief The state of the GPIO pin when the LED is on. If the pin is 
    /// attached to the LED cathode then this is LOW (the default).
    int _onState; 

    /// @brief The current state of the LED
    led_state_t _ledState = LED_OFF;

    /// @brief Calculates the dutycycle to be used for the LED PWM channel, with
    /// consideration of the [brightness] and [onState] values.
    /// @return A dutycycle as 8-bit unsigned integer.
    int _dutyCycle(int brightness);

    /// @brief The pattern to use if the LED is on.
    uint16_t _onPattern[2] = {250, 0};

};


#endif // __GPIO_LED_H__