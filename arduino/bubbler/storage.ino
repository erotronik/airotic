// Storage has two separate implementations for the Itsy Bitsy and for the ESP32
// On the Itsy Bitsy, we use a FAT filesystem, on the ESP32 we use LittleFS.

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

// on ESP32 we use LittleFS

#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>

#define flashfile_FILENAME "/x0.txt"

#define FORMAT_LITTLEFS_IF_FAILED true

struct data_format {
  int version = 1;
  CHSV colorTarget;
  CHSV colorStart;
  short bottle_number;
};

void storage_setup() {
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    Serial.println("LittleFS Mount Failed");
    return;
  }
  Serial.println("LittleFS Mount Succeeded");

  data_format df;

  File data = LittleFS.open(flashfile_FILENAME);
  if (data && !data.isDirectory()) {
    Serial.println("Reading from storage");
    auto read = data.read((uint8_t*)&df, sizeof(df));
    if (read != sizeof(df)) {
      Serial.println("Storage read failed");
      return;
    }
    if (df.version != 1) {
      Serial.println("Invalid data format version");
      return;
    }
    bottle_number = df.bottle_number;
    colorTarget = df.colorTarget;
    colorStart = df.colorStart;
    data.close();
  } else {
    storage_write();
    Serial.println("set defaults, now resetting");
    LittleFS.end();
    delay(1000);
    ESP.restart();
  }
  LittleFS.end();
}

void storage_write() {
  if(!LittleFS.begin(false)){
    Serial.println("LittleFS Mount Failed");
    return;
  }
  auto flashfile = LittleFS.open(flashfile_FILENAME, FILE_WRITE, true);
  if (!flashfile) {
    Serial.println("Could not open file for writing");
    return;
  }
  Serial.println("Writing to storage");
  data_format df;
  df.bottle_number = bottle_number;
  df.colorTarget = colorTarget;
  df.colorStart = colorStart;
  auto written = flashfile.write((uint8_t*)&df, sizeof(df));
  if (written != sizeof(df)) {
    Serial.println("Number of written bytes incorrect");
    return;
  }
  flashfile.flush();
  flashfile.close();
  LittleFS.end();
}

#endif
