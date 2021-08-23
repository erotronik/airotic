# Bubbler Bottle electronics and programming

## Parts list

* Adafruit Itsy Bitsy NRF52480 Express [(uk: Pimoroni)](https://shop.pimoroni.com/products/adafruit-itsybitsy-nrf52840-express-bluetooth-le)
* BMP280 Barometric pressure sensor [(uk: various ebay sellers)](https://www.ebay.co.uk/itm/283518687928)
* 2 pairs of micro 3 pin JST 1.25 connectors (or any small 3 pin connectors or just hard wire instead) [(uk: various ebay sellers)](https://www.ebay.co.uk/itm/301857866521)
* one or two M2.5 5mm screws
* Two WS2812b (neopixels) on small round PCB [(uk: various ebay sellers)](https://www.ebay.co.uk/itm/203167394144)
* A tiny bit of double sided foam tape (or hotglue)

## Construction

* [Construction guide](./Soldering.md)

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

## Connecting via bluetooth

to come

