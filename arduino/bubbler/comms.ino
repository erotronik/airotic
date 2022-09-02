#ifdef ENABLE_BLE

// https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers steal a name nothing over 0x1000 is used anyway
#define our_fake_company_id0 0xf1
#define our_fake_company_id1 0xf1

#include "coyote.h"

#ifndef ESP32

BLEUart bleuart;

void comms_init(short myid) {
  char buf[15];
  snprintf(buf, 15, "Air%02d", myid);
  Bluefruit.begin(1, 1);
  //Serial.println(NRF_FICR->DEVICEADDR0);
  Bluefruit.setTxPower(4);
  Bluefruit.setName(buf);
  Bluefruit.Periph.setConnectCallback(comms_connect_callback);
  bleuart.begin();

  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterRssi(-128);
  Bluefruit.Scanner.setInterval(400, 200);   // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(false); // only need an rssi
  Bluefruit.Scanner.start(0); // 0 = Don't stop scanning after n seconds

  Bluefruit.Central.setDisconnectCallback(central_disconnect_callback);
  Bluefruit.Central.setConnectCallback(central_connect_callback);

  comms_start_adv();
  coyote_setup();
}

void comms_start_adv(void)
{
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addService(bleuart);

  // max is 7 bytes we can fit in, so assume "v" is always 255
  uint8_t msd_payload[7] = {our_fake_company_id0, our_fake_company_id1,
                            bottle_number,
                            colorTarget.h, colorTarget.s,
                            colorStart.h, colorStart.s
                           };
  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, msd_payload, sizeof(msd_payload));
  Bluefruit.ScanResponse.addName();
  /* Start Advertising
     - Enable auto advertising if disconnected
     - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     - Timeout for fast mode is 30 seconds
     - Start(timeout) with timeout = 0 will advertise forever (until connected)
     For recommended advertising interval
     https://developer.apple.com/library/content/qa/qa1931/_index.html
  */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void comms_connect_callback(uint16_t conn_handle)
{
  BLEConnection* connection = Bluefruit.Connection(conn_handle);
  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));
  btAndSerialPrintf("Connected to %s",central_name);
}

void scan_callback(ble_gap_evt_adv_report_t *report)
{
  uint8_t buffer[32];
  uint8_t len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buffer, sizeof(buffer));

  auto res = check_scan_data((char*)buffer, len, report->rssi);

  if ( res == Coyote )
    Bluefruit.Central.connect(report);

  Bluefruit.Scanner.resume();
}

#else /* ESP32 */

#include <NimBLEDevice.h>

NimBLEServer *pServer = NULL;
NimBLECharacteristic * pTxCharacteristic;
bool device_connected = false;
NimBLEScan* pBLEScan;
int scanTime = 5; //In seconds

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class BubblerServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
      Serial.println("Device connected");
      device_connected = true;
    };

    void onDisconnect(NimBLEServer* pServer) {
      Serial.println("Device disconnected");
      device_connected = false;
      // we need to manually restart advertising
      pServer->getAdvertising()->start();
    }
};

class BubblerCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0)
        buffer.append(rxValue);
    }

public:
    bool has_data() {
      return !buffer.empty();
    }

    int get_char() {
      if (buffer.empty())
        return 0;

      int out = buffer[0];
      buffer.erase(0,1);
      return out;
    }

    std::string buffer;
};

bool client_connected = false;
NimBLEAdvertisedDevice* coyote_device = nullptr;


class BubblerAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
      //Serial.printf("Advertised Device: %s \n", advertisedDevice->toString().c_str());
      auto res = check_scan_data(advertisedDevice->getManufacturerData().c_str(), advertisedDevice->getManufacturerData().length(), advertisedDevice->getRSSI());
      if ( res == Coyote ) {
        // can't connect while scanning is going on - it locks up everything.
        coyote_device = new NimBLEAdvertisedDevice(*advertisedDevice);
        NimBLEDevice::getScan()->stop();
      }
    }
};

BubblerCallbacks callbacks;

void comms_init(short myid) {
  char buf[15];
  snprintf(buf, 15, "Air%02d", myid);
  NimBLEDevice::init(buf);
  // BLEDevice::setPower(ESP_PWR_LVL_P9);

  pBLEScan = NimBLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new BubblerAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(2000);
  pBLEScan->setWindow(2000);  // less or equal setInterval value

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new BubblerServerCallbacks());
  comms_start_adv();
  coyote_setup();
}

void scan_loop() {
  if ( !client_connected ) {
    NimBLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.print("Devices found: ");
    Serial.println(foundDevices.getCount());
    Serial.println("Scan done!");
    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    delay(2000);

    if ( coyote_device ) {
      connect_to_coyote(coyote_device);
      delete coyote_device;
      coyote_device = nullptr;      
    }
  }
}

void comms_start_adv(void) {
  // Create the BLE Service
  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      NIMBLE_PROPERTY::NOTIFY
                      );

  NimBLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID_RX,
                                          NIMBLE_PROPERTY::WRITE
                                          );

  pRxCharacteristic->setCallbacks(&callbacks);

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}

#endif

void comms_send_breath(boolean b) {
  btAndSerialPrintf("*%c\n",b?'B':'R');
#ifndef ESP32
  coyote_cb(b);
#endif
}

void comms_uart_send_bottles() {
  if (millis() / 100 % 5 != 0) return;

  for (short i = 0; i < MAX_BOTTLES; i++) {
     if (seen_bottles[i].rssi_av<128) {
       btAndSerialPrintf("%d\r\n",seen_bottles[i].rssi_av);
     }
  }
}

boolean comms_uart_send_graph(int pread, int avg) {
  btAndSerialPrintf("%d,%d\r\n",pread,avg);
  return true;
}

short last_button = 0;

bool bluetooth_available() {
#ifndef ESP32
  return Bluefruit.connected() && bleuart.notifyEnabled();
#else
  return callbacks.has_data();
#endif
}

int bluetooth_read() {
#ifndef ESP32
  return bleuart.read();
#else
  auto data = callbacks.get_char();
  Serial.printf("Got: %c\n", data);
  return data;
#endif
}

void comms_uart_colorpicker(void) {
  boolean bt = false;
  // Respond to the Adafruit client so we don't need to bother to write one, plus extra commands
  //
  // !C[RGB] - set color of LED selected by below
  // !B1 - LED to change is the target (default)
  // !B2 - LED to change is the start
  // !B3 - Use default LEDs
  // !B4 - reboot after setting options so we advertise correctly
  // !S[0-9][0-9] - Set bottle ID
  // !D[0-9] - Debug mode (&1 is "breath sensor graph")(&2 is "rssid")
  //
  bt = bluetooth_available();
  if (!bt && Serial.available()<1)
    return;

  Serial.println("Got something");

  int command = bt?bluetooth_read():Serial.read();
  if (command != '!')
     return;

  command = bt?bluetooth_read():Serial.read();
  if (command == 'H') {
      btAndSerialPrintf("*Hello I am bottle %d\n", bottle_number);
  }
  else if (command == 'C') {
    uint8_t r = bt?bluetooth_read():Serial.read(); // TODO: might want to do this different for serial
    uint8_t g = bt?bluetooth_read():Serial.read();
    uint8_t b = bt?bluetooth_read():Serial.read();
    CRGB x = CRGB(r, g, b);
    if (last_button == 0 || last_button == 1) {
      colorTarget = rgb2hsv_approximate(x);
      colorMyTarget = colorTarget;
      px.fill( px.Color(x.r, x.g, x.b), 0, NUM_LEDS );
      px.show();
      //fill_solid( leds, NUM_LEDS, colorTarget );
      //FastLED.show();
    } else if (last_button == 2) {
      colorStart = rgb2hsv_approximate(x);
      colorMyStart = colorStart;
      CRGB start = colorStart;
      px.fill( px.Color(start.r, start.g, start.b), 0, NUM_LEDS );
      px.show();
      //fill_solid( leds, NUM_LEDS, colorStart );
      //FastLED.show();
    }
    storage_write();
  } else if (command == 'B') {
    command = bt?bluetooth_read():Serial.read();
    if (command == '4') {
      btAndSerialPrintf("rebooting\n");
      delay(100);
#ifndef ESP32
      NVIC_SystemReset();
#else
      ESP.restart();
#endif
    }
    last_button = command - '0';
    if (last_button == 3) {
      colorTarget = colorTargetDefault;
      colorStart = colorStartDefault;
      storage_write();
      btAndSerialPrintf("ok\n");
    }
  } else if (command == 'S') {
    command = bt?bluetooth_read():Serial.read();
    bottle_number = (command - '0')*10;
    command = bt?bluetooth_read():Serial.read();
    bottle_number += (command - '0');
    if (bottle_number < MAX_BOTTLES) {
      storage_write();
      btAndSerialPrintf("ok, set, rebooting\n");
      delay(100);
#ifndef ESP32
      NVIC_SystemReset();
#else
      ESP.restart();
#endif
    } else {
      btAndSerialPrintf("bad number\n");
    }
  } else if (command == 'D') {
    command = bt?bluetooth_read():Serial.read();
    debug_mode = command - '0';
    btAndSerialPrintf("ok\n");
  }
}

enum scan_callback_result check_scan_data(const char* ble_manufacturer_specific_data, int length, int rssi) {
  if (length > 2 && ble_manufacturer_specific_data[1] == 0x19 && ble_manufacturer_specific_data[0] == 0x96) {
    //
    // Found a Coyote
    //
    btAndSerialPrintf("Found DG-LAB\n");
    return Coyote;
  } else if (length > 2 && ble_manufacturer_specific_data[1] == our_fake_company_id1 && ble_manufacturer_specific_data[0] == our_fake_company_id0) {
    //
    // Found a Bottle
    //
    short bottleno = ble_manufacturer_specific_data[2] & 0xf;
    if (bottleno&1 == 1) {  // we only look at odd id bottles
      if (seen_bottles[bottleno].rssi_av == 128) {
        btAndSerialPrintf("How do you do fellow bottle #%d (%d)\n", bottleno, 0 - rssi);
        seen_bottles[bottleno].rssi_av = 0 - rssi;
        seen_bottles[bottleno].colorTarget = CHSV(ble_manufacturer_specific_data[3], ble_manufacturer_specific_data[4], 255);
        seen_bottles[bottleno].colorStart = CHSV(ble_manufacturer_specific_data[5], ble_manufacturer_specific_data[6], 255);
      }
      seen_bottles[bottleno].rssi_av = (seen_bottles[bottleno].rssi_av * 7 + (0 - rssi) * 3) / 10;
      seen_bottles[bottleno].rssi_last = millis();
    }
    return Bottle;
  }
  return None;
}

long rssi_last = 128;

void comms_check_distance(int distance) {
  //
  // not seen for more than 10 seconds? set rssid to 128
  //
  for (short i = 0; i < MAX_BOTTLES; i++) {
    if (seen_bottles[i].rssi_av != 128 && (millis() - seen_bottles[i].rssi_last > 10000)) {
      seen_bottles[i].rssi_av = 128;
      btAndSerialPrintf("Bottle %d has departed\n", i);
    }
    if (seen_bottles[i].rssi_av <= distance && seen_bottles[i].state == 0) {
      btAndSerialPrintf("Bottle %d is close %d\n", i, seen_bottles[i].rssi_av);
      seen_bottles[i].state = 1;
      seen_bottles[i].changed_state = true;
    }
    if (seen_bottles[i].rssi_av >= (distance + 6) && seen_bottles[i].state == 1) {
      btAndSerialPrintf("Bottle %d is away %d\n", i, seen_bottles[i].rssi_av);
      seen_bottles[i].state = 0;
      seen_bottles[i].changed_state = true;
    }
  }
}

void btAndSerialPrintf(const char* format, ...) {
  va_list args;
  va_start(args, format);

  int bufferSize = vsnprintf(NULL, 0, format, args)+1;
  char* buffer = new char[bufferSize];

  vsnprintf(buffer, bufferSize, format, args);

#ifndef ESP32
  if (Bluefruit.connected())
      bleuart.print(buffer);
#else
  if (device_connected) {
    pTxCharacteristic->setValue((uint8_t*)buffer, strlen(buffer));
    pTxCharacteristic->notify();
  }
#endif

  Serial.print(buffer);

  va_end(args);

  delete(buffer);
}

#endif /* ENABLE_BLE */
