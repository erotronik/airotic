# Bubbler Bottle electronics and programming

## Parts list

|Item|UK Seller|USA Seller|
|--|--|--|
| Adafruit Itsy Bitsy NRF52480 Express |[Pimoroni](https://shop.pimoroni.com/products/adafruit-itsybitsy-nrf52840-express-bluetooth-le)| [Adafruit](https://www.adafruit.com/product/4481) |
| BMP280 Barometric pressure sensor |[various ebay sellers](https://www.ebay.co.uk/itm/283518687928)|[various ebay sellers](https://www.ebay.com/itm/133104662930)|
| 2 pairs of micro 3 pin JST 1.25 connectors (or any small 3 pin connectors or just hard wire instead) |[various ebay sellers](https://www.ebay.co.uk/itm/301857866521)|[Adafruit](https://www.adafruit.com/product/4721)|
| Two WS2812b (neopixels) on small round PCB|[various ebay sellers](https://www.ebay.co.uk/itm/203167394144)|[Adafruit](https://www.adafruit.com/product/1612)
| A tiny bit of double sided foam tape (or hotglue)||[Adafruit](https://www.adafruit.com/product/5019)|
| one M2.5 5mm screw|any hardware store or ebay|

Note that it is now possible to use an ESP32 instead of the Adafruit Itsy Bitsy; see this [separate readme](ESP32.md).

## Construction

### Step 1

<a href="b1.jpg"><img src="b1.jpg" width="800"></a>

* Shorten 2 male and 1 female JST leads to around 60mm on all 3 connectors, strip, and tin
* Shorten the other female JST lead, black to around 55mm, red and yellow to around 40mm, strip
* Add thin wires to the BMP280 module so the wires come out on the blank side not the component side.  Trim the solder joints so they are not too prominent.  Solder wires about 100mm long to +v and gnd, and wires about 90mm long to SDA and SCL
* Tin the connections on the LED modules to aid assembly.  Do a test fit and make sure they fit in the cap.

### Step 2

<a href="b2.jpg"><img src="b2.jpg" width="300"></a>

* Feed a male JST through the left hole (left when the flat mounting panel facing you). Solder it to one of the neopixels. Yellow is to DIN, Red is +5V, Black to gnd.  There are no connections to the other 3 pins.

### Step 3

<a href="b3.jpg"><img src="b3.jpg" width="300"></a>

* Feed the 60mm-lead female JST through the right hole. Solder it to the other neopixel. Yellow is DOUT, Red is +5v, black to gnd.

### Step 4

<a href="b4.jpg"><img src="b4.jpg" width="300"></a>

* Feed the remaining male JST lead through the same hole and solder it to the other side of the neopixel. Yellow is DIN, Red is +5v, black to gnd. Solder it so the leads all exit from the same side if possible.  Check your connections carefully make sure DIN and DOUT are not touching.

### Step 5

<a href="b5.jpg"><img src="b5.jpg" width="400"></a>

* Carefully push the LEDs back up into the holes for them and bend the wires so they fit as flat as possible. You could install the LED covers at this point to keep them in place.

### Step 6

<a href="b6.jpg"><img src="b6.jpg" width="800"></a>

* Solder the remaining JST female and BCM280 board to the Itsy Bitsy.  Start with the GND wire as you need to fit two wires into the same hole.  With thin enough wires this should be fairly easy. Then the JST female red wire goes to the VHI pin, and the JST socket yellow wire goes to the "5!" pin.

* Solder the other wires from the BCM280 board. The red leads goes to 3V, SCL to SCL, and finally SDA to SDA.

### Step 7

<a href="b7.jpg"><img src="b7.jpg" width="400"></a>

* You can mount the BMP280 board over the air hole using the M2.5 screw. Ensure the component side of the board is against the hole with the leads at the top as shown.  Important: make sure the sensor (the 2mm square silver part) is facing inwards, against the hole.
* Connect all the JST cables and bend/tuck them out of the way.

### Step 8

<a href="b8.jpg"><img src="b8.jpg" width="400"></a>

* Secure the Itsy Bitsy board with a bit of double sided foam tape (or hotglue)

That's it, now you just need to program and test.
