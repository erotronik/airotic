#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;

void airsensor_setup() {
  if (!bmp.begin(0x76, BMP280_CHIPID)) {
    Serial.println("Missing BMP280");
    //while(1) yield();
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_NONE, /* No temperature */
                  Adafruit_BMP280::SAMPLING_X1, /* Pressure, no oversampling */
                  Adafruit_BMP280::FILTER_OFF, /* No Filtering */
                  Adafruit_BMP280::STANDBY_MS_63); /* Standby time */
}

int airsensor_read() {
  return bmp.readPressure();
}
