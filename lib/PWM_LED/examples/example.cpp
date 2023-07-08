/*!
* @file /*!
* @file main.cpp
*
* @mainpage Sketch to demonstrate the PWM_LED library.
*
* @section intro_sec_Introduction
*
* This sketch demontrates how to control an LED using the PWM_LED class. 
* It also shows how changing the brightness and flashing pattern can be
* accomplished with very little effort.
*
* This sketch requires an LED connected to pin 16. The
* LED is associated with an instance of the PWM_LED class and
* driven by PWM channel 0. The `brightness` is passed by reference 
* to the PWM_LED instance. Changing the value of 
* `brightness` changes the brightness of the LED. Changing any element
* of `pattern` changes the flashing pattern of the LED.
*
* The hardware setup is as follows: 
* - The LED cathode is connected to ground via a voltage limiting 
*   resistor; and
* - The LED anode is connected to GPIO 16.
*
* See [this tutorial](https://www.google.com/search?q=random+nerd+pwm&oq=random+nerd+pwm&aqs=edge..69i57j0i546j0i546i649j69i60l2.5334j0j1&sourceid=chrome&ie=UTF-8)
* For a description of the hardware setup
*
* The PWM_LED instance is initialized in the `setup()` routine. To test 
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
* the end of every loop until they reach 2.5 seconds, at which point they 
* reset to 10mS.
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

#include <PWM_LED.h>

// Connect LED to pin 16.
#define LED_PIN GPIO_NUM_16
#define LED_PWM 0U

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
///
/// The hardware setup is as follows: 
/// - The LED cathode is connected to ground via a voltage 
///   limiting resistor.
/// - The LED anode is connected to GPIO 16.
/// 
/// The PWM_LED instance uses PWM channel 0 and the `brightness`
/// is passed in by reference. The start-up `brighness` is 100% (0XFF).
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
  delay (2500);          // keep on for 2.5 seconds
  brightness = 64;       // dim to 25%
  delay(2500);           // wait 2.5 seconds
  LED.off();             // turn LED off
  
  // debug message
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
  brightness = (int)((float)(brightness) / 2);
  // roll over brightness at zero
  brightness = brightness <= 1 ? 0xff : brightness;
  
  // double the dot lengths until they are 2.5 seconds long
  pattern[0] = pattern[0] > 2500? 10: pattern[0] * 2;;
  pattern[4] = pattern[0];
  
}
