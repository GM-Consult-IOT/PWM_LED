/*!
* @file /*!
* @file main.cpp
*
* @mainpage Sketch to demonstrate the PWM_LED library.
*
* @section intro_sec_Introduction
*
* This sketch requires an RGB LED connected to pins 14, 27 and 12. The
* LEDs are associated with three instances of the PWM_LED class and
* driven by PWM channels 2, 3 and 4 respectively. The three LEDs all
* use the same `brightness` value, passed by reference to the PWM_LED
* instances. Changing the value of `brightness` changes the brightness
* of all three LEDs.
*
* The PWM_LED instances are initialized in the `setup()`
* routine and then turned on for 1 second, one after the other.
*
* During the loop() task the LEDs are activated as follows:
* - the RED LED is turned on for 1.5 seconds and then turned off.
* - the GREEN LED is turned on for 1 second and then turned off.
* - the BLUE LED is flashed in a dot-dash-dot (. - .) pattern for 
*   5 seconds and then turned on.
*
*
* The brightness is halved at the end of every loop and rolls over 
* at or below 1.
* 
* The length of the first and last flash of the pattern is doubled at
* the end of every loop until they reach 2.5 seconds, at which point they 
* reset to 10mS.
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

#include <PWM_LED.h>

// Connect 3 LEDs (or an RGB LED) to pins 14, 27 and 12.

#define LED_RED_PIN 14U
#define LED_GREEN_PIN 27U
#define LED_BLUE_PIN 12U

#define LED_RED_PWM 2
#define LED_GREEN_PWM 3
#define LED_BLUE_PWM 4

#define DOT 100U
#define OFF 100U
#define DASH 500U
#define BREAK 1000U

/// @brief Create a dot - dash - dot flashing pattern
uint16_t pattern[] = {DOT,OFF,DASH,OFF,DOT,BREAK};

/// @brief The variable that holds the brightness value, passed
/// by reference to the PWM_LED instance.
int brightness = 0xff;

// instantiate the PWM_LED instances.
PWM_LED red(LED_RED_PIN, LED_RED_PWM, brightness, HIGH);
PWM_LED green( LED_GREEN_PIN, LED_GREEN_PWM, brightness, HIGH);
PWM_LED blue(LED_BLUE_PIN, LED_BLUE_PWM, brightness, HIGH);

// get everything ready
void setup() {
  // set up the debug port
  Serial.begin(115200);
  while (!Serial){
    vTaskDelay(50/portTICK_PERIOD_MS);
  };
  
  // handshake
  Serial.println("Up and running!");

  // initialize the LED instances
  red.begin();
  blue.begin();
  green.begin();

  // test the LEDs are working
  red.on();
  delay(500);
  red.off();
  green.on();
  delay (500);
  green.off();
  blue.on();
  delay (500);
  // set the brightness around 25%
  brightness = 64;
  Serial.println("setup() done!");
  delay(1000);            // wait one second  
  blue.off();             // turn BLUE off
}

void loop() {  

  // print the brightness to the debug port
  Serial.printf("Brightness is %S percent (%u)\n", 
    String(double(brightness) / 0xff * 100, 0), 
    brightness);
  
  // do a bit of turning on and off and flashing
  delay(1000);            // wait one second
  green.on();             // turn GREEN on   
  delay(2000);            // keep GREEN on for 2 seconds
  green.off();            // turn GREEN off
  delay(1000);            // wait one second
  red.on();               // turn RED on
  delay(2000);            // keep RED on for 2 seconds
  red.off();              // turn RED off
  delay(1000);            // wait one second
  blue.flash(pattern, 6); // flash dot-dash-dot pattern on BLUE
  delay(5000);            // keep flashing for 5 seconds
  blue.off();             // turn BLUE off
  // wait for the the blue LED state to become LED_OFF to avoid having multiple
  // LEDs on.
  while (blue.state()!=LED_OFF){
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
  delay(pattern[0]*2);    // make sure the last dot has completed 
 
  // halve the brightness
  brightness = (int)((float)(brightness) / 2);
  
  // roll over brightness at zero
  brightness = brightness <= 1 ? 0xff : brightness;
  // double the dot lengths until they are 2.5 seconds long
  pattern[0] = pattern[0] > 2500? 10: pattern[0] * 2;;
  pattern[4] = pattern[0];
  
}
