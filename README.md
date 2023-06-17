# GPIO_LED
Control an LED directly from a GPIO pin.

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

The following example is also in the `examples folder`

## References

