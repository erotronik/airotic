# Bubbler Bottle programming

## Programming setup

* Install the arduino tool and board support. [Use this Adafruit guide](https://learn.adafruit.com/adafruit-itsybitsy-nrf52840-express/arduino-support-setup)
* In the arduino tool goto "Tools" "Manage Libraries"
  * Search for "FastLED" and install it
  * Search for "BMP280" and install "Adafruit BMP280 Library"
  * Search for "spiflash" and install "Adafruit_SPIFlash"

## Programming first step

* Select the board "ItsyBitsy nRF52840 Express"
* First you need to format the storage we use.  You should only ever need to do this once.  So open the examples "Adafruit SPIFlash" "SdFat_format", write it to the board, open the serial console, and select OK to flash.

## Programming

* Download the Bubbler code and upload it to the board.  On the first run the code will store some defaults and reboot, so it will take a bit longer

* On startup the code will cause both the LEDs to light up white, then shortly after they'll turn pink then fade to red. Blowing hard around the sensor hole will cause them to turn pink and fade back to red again.

Now you can connect to the board via bluetooth and set some defaults if you wish

## Troubleshooting

* On startup both LEDs should turn white briefly. If only one turns white check the connections between the LEDs.

* If the blowing over the sensor doesn't cause anything to happen, look at the serial console in the Arduino
application when starting up. It may be that the sensor isn't being detected. This could be because of a bad
sensor (out of the 6 we bought 1 was faulty) or the connections are bad (check SDA and SCL are the right way
around)

