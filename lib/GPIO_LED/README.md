# PWM_LED

Control an LED's state, flashing pattern and brightness using PWM on a GPIO pin.

## Contents
- [GPIO\_LED](#gpio_led)
  - [Contents](#contents)
  - [Overview](#overview)
  - [Usage](#usage)
  - [References](#references)

## Overview

This library provides an interface to control an LED connected to a GPIO pin using PWM. The principle of operation as as follows:
* the LED is controlled by writing a PWM signal to the specified GPIO pin using the nominated PWM channel.
* the library calculates the PWM signal from the `brightness` value and whether the `onState` of the LED is `HIGH` or `LOW`.
* in addition to the ability to turn the LED on or off, a flashing pattern can be provided by calling the `flash(pattern, length)` method. The pattern is a simple array sequence of milliSecond timings in which the even-index elements (elements 0, 2, 4 ...) are the `on` periods and the odd-index elements are the `off` periods in milliseconds. The pattern length is limited to 255 elements.

The PWM output is managed by a FreeRTOS task with a fairly low priority (task priority 10), which means the flashing of the LED runs asynchronously (non-blocking).

## Usage

*The following example is also in the [examples folder](https://github.com/GM-Consult-IOT/PWM_LED/tree/main/lib/PWM_LED/examples).*

This sketch demontrates how to control an LED using the PWM_LED. It also shows how changing the brightness and flashing pattern can be accomplished with very little effort.

This sketch assumes an LED's anode is directly connected to pin 27. The LED is associated with an instance of the PWM_LED class and driven by PWM channel 0. 

The `brightness` is passed by reference to the PWM_LED instance. Changing the value of `brightness` changes the brightness of the LED. Changing any element of `pattern` changes the flashing pattern of the LED.

The hardware setup is as shown in [this tutorial](https://www.google.com/search?q=random+nerd+pwm&oq=random+nerd+pwm&aqs=edge..69i57j0i546j0i546i649j69i60l2.5334j0j1&sourceid=chrome&ie=UTF-8):
* the LED cathode is connected to ground via a voltage limiting resistor; and
* the LED anode is connected to GPIO 16.

The PWM_LED instance is initialized in the `setup()` routine. To test the hardware, the LED is turned on for 1 second and then dimmed to about 25%.

During the loop() task the LED is activated as follows: 
* the LED is turned on for 2 seconds and then turned off for a second; and
* the LED is flashed in a dot-dash-dot (. - .) pattern for 5 seconds and then turned off.

The brightness is halved at the end of every loop and rolls over at or below 1. 

The length of the first and last flash of the pattern is doubled at the end of every loop until they reach 2.5 seconds, at which point they reset to 10mS.

``` C++

#include <PWM_LED.h>

// Connect LED to pin 16.
#define LED_PIN GPIO_NUM_16
// Use PWM channel 0
#define LED_PWM 0U

// Elements of the flashing pattern
#define DOT 100U
#define OFF 100U
#define DASH 500U
#define BREAK 1000U

/// @brief Create a dot - dash - dot flashing pattern
uint16_t pattern[] = {DOT,OFF,DASH,OFF,DOT,BREAK};

/// @brief The variable that holds the brightness value, passed
/// by reference to the PWM_LED instance.
int brightness = 0xff;

/// @brief Instantiate the PWM_LED instance.
PWM_LED LED(LED_PIN, LED_PWM, brightness, HIGH);

// get everything ready
void setup() {

  // set up the debug port
  Serial.begin(115200);
  
  // wait for Serial to be initialized
  while (!Serial){
    vTaskDelay(50/portTICK_PERIOD_MS);
  };
  
  // send handshake
  Serial.println("Up and running!");

  // initialize the LED, starting the PWM task
  LED.begin();

  // test the LED is working
  LED.on();              // turn on the LED
  delay (2500);          // keep on for 2.5sec
  brightness = 75;      // dim to 50%
  delay(2500);           // wait one second  
  LED.off();             // turn LED off
  Serial.println("setup() done!");
}

void loop() {  

  // print the brightness to the debug port
  Serial.printf("Brightness is %S percent (%u)\n", String(double(brightness) / 0xff * 100, 0), brightness);
  
  // do a bit of turning on and off and flashing
  delay(1000);            // wait one second
  LED.on();             // turn LED on   
  delay(2000);            // keep LED on for 2 seconds
  LED.off();            // turn LED off
  delay(1000);            // wait one second
  LED.flash(pattern, 6); // flash dot-dash-dot pattern on LED
  delay(5000);            // keep flashing for 5 seconds
  LED.off();             // turn LED off
 
  // halve the brightness
  brightness -= (int)((float)(brightness) / 2);
  dot = dot > 2500? 10: dot * 2;
  pattern[4] = dot;
  pattern[0] = dot;
  // roll over brightness at zero
  brightness = brightness <= 1 ? 0xff : brightness;
  
}

```

## References
* [ESP32 PWM with Arduino IDE](https://www.google.com/search?q=random+nerd+pwm&oq=random+nerd+pwm&aqs=edge..69i57j0i546j0i546i649j69i60l2.5334j0j1&sourceid=chrome&ie=UTF-8)
* [FreeRTOS](https://freertos.org/index.html)

