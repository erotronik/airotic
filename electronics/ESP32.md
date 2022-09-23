# Using an ESP32 for the Bubbler

Instead of the "Adafruit Itsy Bitsy NRF52480 Express", you also can use an ESP32, specifically an "Adafruit QT Py ESP32-C3" ([Adafruit (US)](https://www.adafruit.com/product/5405) / [Pimoroni (UK)](https://shop.pimoroni.com/products/adafruit-qt-py-esp32-c3-wifi-dev-board-with-stemma-qt?variant=39850337370195)).

Note - it is possible that you can also use other ESP32 boards with the same form factor. However, we only tested this specific board.

To use this board, use the [main construction manual](README.md) with the following changes:

To solder the JST and the BMP280 boards to the ESP32, start with the GND wire, as you need to fit two wires into the same hole. The JST female red wire goes to the "5V" pin, the jst socket yellow wire goes to the "A0" PIN.

Solder the other wires from the BCM280 board. VCC on the BMP280 goes to the "3V" pin, SDA goes to the "SDA" pin and SCL goes to the "SCL" pin.

Since the Adafruit QT Py has a Stemma QT port, you can also solder the BMP280 to a 4-pin JST-ST/Stemma QT cable, and plug that cable into the Qt Py. Suitable cables are available at [Adafruit (US)](https://www.adafruit.com/product/4399) / [Pimorino (UK)](https://shop.pimoroni.com/products/jst-sh-cable-qwiic-stemma-qt-compatible?variant=31910609813587). You will probably have to get a cable with two JST-SH ports, and cut it apart.