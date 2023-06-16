#include <Arduino.h>
#include <GPIO_LED.h>

// Connect 3 LEDs (or an RGB LED) to pins 14, 27 and 12.

#define LED_RED_PIN 14U
#define LED_GREEN_PIN 27U
#define LED_BLUE_PIN 12U

#define LED_RED_PWM 2
#define LED_GREEN_PWM 3
#define LED_BLUE_PWM 4

/// @brief Create a dot - dash - dot flashing pattern
uint16_t pattern[] = {100,50,500,50,100,250};

// instantiate the GPIO_LED instances.
GPIO_LED red(COLOR_RED, LED_RED_PIN, LED_RED_PWM, HIGH);
GPIO_LED green(COLOR_GREEN, LED_GREEN_PIN, LED_GREEN_PWM, HIGH);
GPIO_LED blue(COLOR_BLUE, LED_BLUE_PIN, LED_BLUE_PWM, HIGH);

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
  delay(1500);;
  red.off();
  green.on();
  delay (1000);
  green.off();
  blue.on();
  delay (1000);
  blue.off();
  Serial.println("setup() done!");
}

uint8_t i = 0;
int brightness = 0xff;

void loop() {  
  // use the counter as brightness
  brightness = i;

  // print the counter and brightness to the debug port
  Serial.printf("Loop no %u, brightness is %S percent\n", i, String(double(brightness) / 0xff * 100, 1));
  
  // do a bit of turning on and off and flashing
  red.off();
  green.off();
  blue.off();
  delay(1000);
  green.on(&brightness);    
  delay(2000);
  green.off();
  red.on(&brightness);
  delay(2000);
  red.off();
  blue.flash(pattern, 6, &brightness);
  delay(2000);
  blue.off();
 
  // increment the counter/brightness
  i += 10;
  
}
