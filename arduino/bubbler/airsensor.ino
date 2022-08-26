#include <Wire.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;

// Track if we do have an airsensor. If no, just return a 0 for read, but otherwise work.
// This lets us, e.e., debug BLE with just a board, even if it does not feature a sensor.
bool have_airsensor = true;

void airsensor_setup() {
  if (!bmp.begin(0x76, BMP280_CHIPID)) {
    Serial.println("Missing BMP280");
    return;
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_NONE, /* No temperature */
                  Adafruit_BMP280::SAMPLING_X1, /* Pressure, no oversampling */
                  Adafruit_BMP280::FILTER_OFF, /* No Filtering */
                  Adafruit_BMP280::STANDBY_MS_63); /* Standby time */
}

int airsensor_read() {
  if ( ! have_airsensor )
    return 0;

  return bmp.readPressure();
}
