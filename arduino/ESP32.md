# ESP32 Bubbler Bottle programming

This is the programming guide if you choose to use an ESP32 instead of the (recommended) ItsyBitsy nRF52840 Express. For the ItsyBitsy, please [see the main readme](README.md).

## Programming setup

* Install the [Arduino IDE](https://www.arduino.cc/en/software) and [the Espressif ESP32 boards](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/installing.html)
* In the Arduino IDE, go to "Tools"->"Manage Libraries"
* Search for, and install the following libraries:
	- FastLED
	- Adafruit NeoPixel
	- NimBLE-Arduino
	- Adafruit BMP280 Library

## Programming
* Select the board "Adafruit QT Py ESP32-C3"
* Download the Bubbler code and upload it to the board. On the first run the code will store some defaults and reboot, so it will take a bit longer.
* On startup the code will cause both the LEDs to light up white, then shortly after they'll turn pink then fade to red. Blowing hard around the sensor hole will cause them to turn pink and fade back to red again.

Now you can connect to the board via bluetooth and set some defaults if you wish

## Wifi Support

You can enable wifi support by changing the `ENABLE_BLE` and `ENABLE_WIFI` defined inside `bubbler.ino`.

Wifi support is quite rough at the moment; if you want to use it you should be familiar with Arduino programming and look at wifi.ino.

To use Wifi, you need to install the following additional libraries using the library manager:
* AsyncMQTT_Generic

You also have to manually install the following library that is not available via the library manager:
* https://github.com/me-no-dev/AsyncTCP