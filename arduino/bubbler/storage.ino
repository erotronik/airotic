#ifndef ESP32

// No EEPROM.h on the itsybitsy so instead use SPI flash and store stuff in a file

#include <SPI.h>
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
File flashfile;

#define flashfile_FILENAME "x0.txt"

void storage_write() {
  flashfile = fatfs.open(flashfile_FILENAME, O_WRITE | O_CREAT);
  Serial.println("Writing to storage");
  flashfile.printf("%d\n%d\n%d\n", colorTarget.h, colorTarget.s, colorTarget.v);
  flashfile.printf("%d\n%d\n%d\n", colorStart.h, colorStart.s, colorStart.v);
  flashfile.printf("%d\n", bottle_number);
  flashfile.close();
}

void storage_setup() {
  flash.begin();
  if (!fatfs.begin(&flash)) {
    Serial.println("Error: no filesystem. Use Adafruit SPIFlash - SdFat_format example to make one.");
    while (1) yield();
  }
  flashfile = fatfs.open(flashfile_FILENAME);
  if (flashfile) {
    Serial.println("Reading from storage");
    colorTarget.h = flashfile.readStringUntil('\n').toInt();
    colorTarget.s = flashfile.readStringUntil('\n').toInt();
    colorTarget.v = flashfile.readStringUntil('\n').toInt();
    colorStart.h = flashfile.readStringUntil('\n').toInt();
    colorStart.s = flashfile.readStringUntil('\n').toInt();
    colorStart.v = flashfile.readStringUntil('\n').toInt();
    bottle_number = flashfile.readStringUntil('\n').toInt();
    colorCurrent = colorTarget;
    colorMyTarget = colorTarget;
    colorMyStart = colorStart;
    flashfile.close();
  } else {
    storage_write();
    Serial.println("set defaults, now resetting");
    delay(1000);
    NVIC_SystemReset();
  }
}

#else

#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>

#endif
