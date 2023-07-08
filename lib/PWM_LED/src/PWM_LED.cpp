/*!
* @file /*!
* @file PWM_LED.h
*
* @mainpage This library provides an interface to control an LED connected 
* to a GPIO pin using PWM.
*
* @section intro_sec_Introduction
*
* This library provides an interface to control an LED connected to a GPIO 
* pin using PWM. The principle of operation as as follows:
* - the LED is controlled by writing a PWM signal to the specified GPIO pin 
*   using the nominated PWM channel.
* - the library calculates the PWM signal from the `brightness` value and 
*   whether the `onState` of the LED is `HIGH` or `LOW`.
* - in addition to the ability to turn the LED on or off, a flashing pattern 
*   can be provided by calling the `flash(pattern, length)` method. The 
*   pattern is a simple array sequence of milliSecond timings in which the 
*   even-index elements (elements 0, 2, 4 ...) are the `on` periods and the 
*   odd-index elements are the `off` periods in milliseconds. 
*   The pattern length is limited to 255 elements.
*
* The PWM output is managed by a FreeRTOS task with a fairly low priority 
* (task priority 10), which means the flashing of the LED runs asynchronously 
* (non-blocking).
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

#include "PWM_LED.h"

#define TASK_STACK_SIZE 0x1000
#define TASK_PRIORITY 10

PWM_LED::PWM_LED(uint8_t pin, 
        uint8_t PwmChannel, 
        int & brightness, 
        int onState):
            _GPIO(pin), 
            _PwmChannel(PwmChannel),
            _brightness(brightness),
            _onState(bool(onState)){};

bool PWM_LED::begin(){
    ledcSetup(_PwmChannel, PWM_LED_PWM_FREQ, PWM_LED_PWM_RESOLUTION);
    ledcAttachPin(_GPIO, _PwmChannel);
    vTaskDelay(100/portTICK_PERIOD_MS);
    if (_createTask()){
        off();        
        _ledState = LED_OFF;
        return true;        
    }
    return false;
};

bool PWM_LED::_createTask(){
    _flashSemaphore = xSemaphoreCreateBinary();
    if (_flashSemaphore == NULL){
        return false;
    } 
    if (!xTaskCreate(this->_flashTaskStatic,
        "LED_TASK",
        TASK_STACK_SIZE,
        this,
        TASK_PRIORITY, 
        &_flashTask)){
        return false;
    } 
    return true;
};

LED_State PWM_LED::state(){
    return _ledState;
};

void PWM_LED::on(){ 
    xSemaphoreTake(_flashSemaphore,  ( TickType_t ) 1);      
    _flashPatternLength = 0;      
    flash(_onPattern,1);
    _ledState = LED_ON;
};

void PWM_LED::off(){  
    xSemaphoreTake(_flashSemaphore,  ( TickType_t ) 1);    
    _flashPatternLength = 0; 
}

void PWM_LED::flash(uint16_t * pattern, uint8_t length){   
    _flashPatternLength = 0; 
    if(length>0){    
        std::copy(pattern, pattern + length, _flashPattern);
        _flashPatternLength = length;
        xSemaphoreGive(_flashSemaphore);
        _ledState = LED_FLASHING;        
    }
}

void PWM_LED::_flash(void){
    #ifdef PWM_LED_DEBUG
    UBaseType_t uxHighWaterMark;
    #endif // PWM_LED_DEBUG    
    ledcWrite(_PwmChannel,_dutyCycle(0));
    uint8_t len;
    uint8_t i;   
    uint32_t timer = millis();
    for (;;){   
        #ifdef PWM_LED_DEBUG
        /* Inspect our own high water mark on entering the task. */
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        Serial.printf("The highwatermark is at 0X%X\n", uxHighWaterMark);
        #endif // PWM_LED_DEBUG    
        if (xSemaphoreTake(_flashSemaphore, portMAX_DELAY)){
            while(_flashPatternLength > 0){  
                i = 0;
                len = _flashPatternLength;              
                timer = millis();
                while (i < len && _flashPatternLength > 0){
                    ledcWrite(_PwmChannel,
                            i % 2 == 0? _dutyCycle(_brightness) :  _dutyCycle(0)); 
                    while (millis()-timer < _flashPattern[i]  && _flashPatternLength > 0){                                                            
                        vTaskDelay(1/portTICK_PERIOD_MS);
                    }
                    timer = millis();
                    i++;
                    #ifdef PWM_LED_DEBUG
                    /* Calling the function will have used some stack space, we would 
                    therefore now expect uxTaskGetStackHighWaterMark() to return a 
                    value lower than when it was called on entering the task. */
                    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
                    // Serial.printf("The highwatermark is at 0X%X\n", uxHighWaterMark);
                    #endif // PWM_LED_DEBUG    
                }
            }            
            ledcWrite(_PwmChannel,_dutyCycle(0));                
            _ledState = LED_OFF;
            i = 0;
        }
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
};

void PWM_LED::_flashTaskStatic(void* _this){
    static_cast<PWM_LED*>(_this)->_flash();
};

int PWM_LED::_dutyCycle(int brightness){
    return _onState == HIGH? brightness: PWM_LED_PWM_MAX_DUTY_CYCLE - brightness;
};