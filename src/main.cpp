/*!
* @file /*!
* @file main.cpp
*
* @mainpage Sketch to demonstrate the GPIO_LED library.
*
* @section intro_sec_Introduction
*
* This sketch demontrates how to control an LED using the GPIO_LED. It
* also shows how changing the brightness and flashing pattern can be
* accomplished with very little effort.
*
* This sketch requires an LED connected to pin 27. The
* LED is associated with an instance of the GPIO_LED class and
* driven by PWM channel 3. The `brightness` is passed by reference 
* to the GPIO_LED instance. Changing the value of 
* `brightness` changes the brightness of the LED. Changing any element
* of `pattern` changes the flashing pattern of the LED.
*
* The hardware setup is as follows: 
* - The LED cathode is connected to ground via a voltage limiting 
*   resistor; and
* - The LED anode is connected to GPIO 27.
*
* The GPIO_LED instance is initialized in the `setup()` routine. To test 
* the hardware, the LED is turned on for 1 second and then dimmed to 
* about 25%.
*
* During the loop() task the LED is activated as follows:
* - the LED is turned on for 2 seconds and then turned off for a second.
* - the LED is flashed in a dot-dash-dot (. - .) pattern for 
*   5 seconds and then turned off.*
*
* The brightness is halved at the end of every loop and rolls over 
* at or below 1.
* 
* The length of the first and last flash of the pattern is doubled at
* the end of every loop until it reaches 2500mS, at which point it resets to 10mS.
* @section author Author
* 
* Gerhard Malan for GM Consult Pty Ltd
* 
 * @section license License
 * 
 * This library is open-source under the BSD 3-Clause license and 
 * LEDistribution and use in source and binary forms, with or without 
 * modification, are permitted, provided that the license conditions are met.
 * 
*/

#include <GPIO_LED.h>

// Connect LED to pin 27.
#define LED_PIN GPIO_NUM_27
#define LED_PWM 3U

#define DOT 100U
#define OFF 100U
#define DASH 500U
#define BREAK 1000U

uint16_t dot = DOT;

/// @brief Create a dot - dash - dot flashing pattern
uint16_t pattern[] = {DOT,OFF,DASH,OFF,dot,BREAK};

/// @brief The variable that holds the brightness value, passed
/// by reference to the GPIO_LED instance.
int brightness = 0xff;

/// @brief Instantiate the GPIO_LED instance.
///
/// The hardware setup is as follows: 
/// - The LED cathode is connected to ground via a voltage 
///   limiting resistor.
/// - The LED anode is connected to GPIO 27.
/// 
/// The GPIO_LED instance uses PWM channel 3 and the `brightness`
/// is passed in by reference. The start-up `brighness` is 100% (0XFF).
GPIO_LED LED(LED_PIN, LED_PWM, brightness, HIGH);

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
