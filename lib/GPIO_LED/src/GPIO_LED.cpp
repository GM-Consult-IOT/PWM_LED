/*!
* @file /*!
* @file GPIO_LED.h
*
* @mainpage Status LED interface for 400 Ocean Series Devices.
*
* @section intro_sec_Introduction
*
* Firmware for the 400 Ocean Series devices. Please see README.md
* for a full description.
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

#include "GPIO_LED.h"

GPIO_LED::GPIO_LED(uint16_t color, 
        uint8_t pin, 
        uint8_t PwmChannel, 
        int onState):
            _color(color), 
            _GPIO(pin), 
            _PwmChannel(PwmChannel),
            _onState(bool(onState)){};

bool GPIO_LED::begin(int brightness){
    ledcSetup(_PwmChannel, GPIO_LED_PWM_FREQ, GPIO_LED_PWM_RESOLUTION);
    ledcAttachPin(_GPIO, _PwmChannel);
    vTaskDelay(100/portTICK_PERIOD_MS);
    _brightness = brightness;    
    _flashSemaphore = xSemaphoreCreateBinary();
    if (_flashSemaphore == NULL){
        #ifdef GPIO_LED_DEBUG
        Serial.println("Failed to create flash semaphore");
        #endif // GPIO_LED_DEBUG
        return false;
    } else {
        #ifdef GPIO_LED_DEBUG
        Serial.println("Flash semaphore created");
        #endif // GPIO_LED_DEBUG
    }
    if (!xTaskCreate(this->_flashTaskStatic,
        "FLASH_TASK",
        4096,
        this,
        10, 
        &_flashTask)){
        #ifdef GPIO_LED_DEBUG
        Serial.println("Failed to create flash task");
        #endif // GPIO_LED_DEBUG
        return false;
    } else {
        #ifdef GPIO_LED_DEBUG
        Serial.println("Flash task created");
        #endif // GPIO_LED_DEBUG
    }
    // ledcWrite(_PwmChannel, GPIO_LED_PWM_MAX_DUTY_CYCLE/2);
    #ifdef GPIO_LED_DEBUG
    Serial.printf("GPIO %u set as PWM output\n", _GPIO);
    ledcWrite(_PwmChannel, GPIO_LED_PWM_MAX_DUTY_CYCLE);
    vTaskDelay(5000);
    #endif // GPIO_LED_DEBUG
    off();
    _ledState = LED_OFF;
    return true;
};

LED_State GPIO_LED::state(){
    return _ledState;
};

void GPIO_LED::setBrightness(int brightness){ 
    if (_ledState == LED_ON) {
        return on(&brightness);
    }
    _brightness = brightness;   
}

void GPIO_LED::on(int * brightness){ 
    _brightness = brightness == NULL? _brightness: brightness[0];
    xSemaphoreTake(_flashSemaphore,  ( TickType_t ) 1);      
    _flashPatternLength = 0;   
    int dutyCycle  = _dutyCycle(_brightness);
    #ifdef GPIO_LED_DEBUG
    Serial.printf("Turned on the LED on GPIO %u, dutycycle %X\n",_GPIO, dutyCycle);
    #endif // GPIO_LED_DEBUG        
    ledcWrite(_PwmChannel,dutyCycle);
    _ledState = LED_ON;
};

void GPIO_LED::off(){  
    xSemaphoreTake(_flashSemaphore,  ( TickType_t ) 1);    
    memset(_flashPattern, 0, sizeof(_flashPattern));
    _flashPatternLength = 0; 
    #ifdef GPIO_LED_DEBUG
    Serial.printf("Turned off the LED on GPIO %u, dutycycle %X\n",_GPIO, _dutyCycle(0));
    #endif // GPIO_LED_DEBUG    
    ledcWrite(_PwmChannel,_dutyCycle(0));    
    _ledState = LED_OFF;
}

uint16_t GPIO_LED::color(){
    return _color;
};

void GPIO_LED::flash(uint16_t * pattern, uint8_t length, int * brightness ){   
    _brightness = brightness == NULL? _brightness: brightness[0]; 
    off();
    if(length>0){
        if (length == 1 && pattern[0]>0){
            on();
        } else {
            std::copy(pattern, pattern + length, _flashPattern);
            _flashPatternLength = length;
            xSemaphoreGive(_flashSemaphore);
            _ledState = LED_FLASHING;
        }
    }
}

void GPIO_LED::_flash(void){
    ledcWrite(_PwmChannel,_dutyCycle(0));
    for (;;){   
        if (xSemaphoreTake(_flashSemaphore, portMAX_DELAY)){
            uint8_t len = _flashPatternLength;
            const uint16_t * pattern = _flashPattern;            
            while(_flashPatternLength > 0){
                for (size_t i = 0; i < len; i++){
                    ledcWrite(_PwmChannel,
                        i % 2 == 0? _dutyCycle(0) : _dutyCycle(_brightness));                                     
                    vTaskDelay(pattern[i]/portTICK_PERIOD_MS);
                }
            }
        }
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
};

void GPIO_LED::_flashTaskStatic(void* _this){
    static_cast<GPIO_LED*>(_this)->_flash();
};


int GPIO_LED::_dutyCycle(int brightness){
    return _onState == HIGH? brightness: GPIO_LED_PWM_MAX_DUTY_CYCLE - brightness;
};