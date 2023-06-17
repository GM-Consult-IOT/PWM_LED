/*!
* @file /*!
* @file PWM_LED.h
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

#include "PWM_LED.h"

#define TASK_STACK_SIZE 0x600
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
    ledcSetup(_PwmChannel, GPIO_LED_PWM_FREQ, GPIO_LED_PWM_RESOLUTION);
    ledcAttachPin(_GPIO, _PwmChannel);
    vTaskDelay(100/portTICK_PERIOD_MS);
    if (_createTask()){
        off();
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
    _ledState = LED_OFF;
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
    #ifdef GPIO_LED_DEBUG
    UBaseType_t uxHighWaterMark;
    #endif // GPIO_LED_DEBUG    
    ledcWrite(_PwmChannel,_dutyCycle(0));
    uint8_t len;
    for (;;){   
        #ifdef GPIO_LED_DEBUG
        /* Inspect our own high water mark on entering the task. */
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        Serial.printf("The highwatermark is at 0X%X\n", uxHighWaterMark);
        #endif // GPIO_LED_DEBUG    
        if (xSemaphoreTake(_flashSemaphore, portMAX_DELAY)){
             len = _flashPatternLength;       
            while(_flashPatternLength > 0){
                for (size_t i = 0; i < len; i++){
                    ledcWrite(_PwmChannel,
                        i % 2 == 0? _dutyCycle(_brightness) :  _dutyCycle(0));                                     
                    vTaskDelay(_flashPattern[i]/portTICK_PERIOD_MS);
                }
                #ifdef GPIO_LED_DEBUG
                /* Calling the function will have used some stack space, we would 
                therefore now expect uxTaskGetStackHighWaterMark() to return a 
                value lower than when it was called on entering the task. */
                uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
                Serial.printf("The highwatermark is at 0X%X\n", uxHighWaterMark);
                #endif // GPIO_LED_DEBUG    
            }            
            ledcWrite(_PwmChannel,_dutyCycle(0));    
        }
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
};

void PWM_LED::_flashTaskStatic(void* _this){
    static_cast<PWM_LED*>(_this)->_flash();
};


int PWM_LED::_dutyCycle(int brightness){
    return _onState == HIGH? brightness: GPIO_LED_PWM_MAX_DUTY_CYCLE - brightness;
};