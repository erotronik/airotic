#ifndef ESP32

BLEUart bleuart;
#include "coyote.h"

// https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers steal a name nothing over 0x1000 is used anyway
#define our_fake_company_id0 0xf1
#define our_fake_company_id1 0xf1

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

void comms_send_breath(boolean b) {
  btAndSerialPrintf("*%c\n",b?'B':'R');
  coyote_cb(b);
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
  
  if ( Bluefruit.connected() && bleuart.notifyEnabled() ) 
    bt = true;
  if (!bt && Serial.available()<1)
    return;

  int command = bt?bleuart.read():Serial.read();
  if (command != '!') 
     return;
     
  command = bt?bleuart.read():Serial.read();
  if (command == 'H') {
      btAndSerialPrintf("*Hello I am bottle %d\n", bottle_number);  
  }
  else if (command == 'C') {
    uint8_t r = bt?bleuart.read():Serial.read(); // TODO: might want to do this different for serial
    uint8_t g = bt?bleuart.read():Serial.read();
    uint8_t b = bt?bleuart.read():Serial.read();
    CRGB x = CRGB(r, g, b);
    if (last_button == 0 || last_button == 1) {
      colorTarget = rgb2hsv_approximate(x);
      colorMyTarget = colorTarget;
      fill_solid( leds, NUM_LEDS, colorTarget );
      FastLED.show();        
    } else if (last_button == 2) {          
      colorStart = rgb2hsv_approximate(x);
      colorMyStart = colorStart;
      fill_solid( leds, NUM_LEDS, colorStart );
      FastLED.show();           
    }
    storage_write();
  } else if (command == 'B') {
    command = bt?bleuart.read():Serial.read();
    if (command == '4') {
      btAndSerialPrintf("rebooting\n");
      delay(100);
      NVIC_SystemReset();
    }
    last_button = command - '0';
    if (last_button == 3) {
      colorTarget = colorTargetDefault;
      colorStart = colorStartDefault;
      storage_write();
      btAndSerialPrintf("ok\n");
    }
  } else if (command == 'S') {
    command = bt?bleuart.read():Serial.read();
    bottle_number = (command - '0')*10;
    command = bt?bleuart.read():Serial.read();
    bottle_number += (command - '0');        
    if (bottle_number < MAX_BOTTLES) {
      storage_write();
      btAndSerialPrintf("ok, set, rebooting\n");
      delay(100);
      NVIC_SystemReset();          
    } else {
      btAndSerialPrintf("bad number\n");
    }
  } else if (command == 'D') {
    command = bt?bleuart.read():Serial.read();
    debug_mode = command - '0';
    btAndSerialPrintf("ok\n");
  }
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

void scan_callback(ble_gap_evt_adv_report_t *report)
{
  uint8_t buffer[32];
  uint8_t len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buffer, sizeof(buffer));

  if (len > 2 && buffer[1] == 0x19 && buffer[0] == 0x96) {
    //
    // Found a Coyote
    //
    btAndSerialPrintf("Found DG-LAB\n");
    Bluefruit.Central.connect(report);
  } else if (len > 2 && buffer[1] == our_fake_company_id1 && buffer[0] == our_fake_company_id0) {
    //
    // Found a Bottle
    //
    short bottleno = buffer[2] & 0xf;
    if (bottleno&1 == 1) {  // we only look at odd id bottles
      if (seen_bottles[bottleno].rssi_av == 128) {
        btAndSerialPrintf("How do you do fellow bottle #%d (%d)\n", bottleno, 0 - report->rssi);
        seen_bottles[bottleno].rssi_av = 0 - report->rssi;
        seen_bottles[bottleno].colorTarget = CHSV(buffer[3], buffer[4], 255);
        seen_bottles[bottleno].colorStart = CHSV(buffer[5], buffer[6], 255);
      }
      seen_bottles[bottleno].rssi_av = (seen_bottles[bottleno].rssi_av * 7 + (0 - report->rssi) * 3) / 10;
      seen_bottles[bottleno].rssi_last = millis();
    }
  }
  Bluefruit.Scanner.resume();
}

void btAndSerialPrintf(const char* format, ...) {
  va_list args;
  va_start(args, format);

  int bufferSize = vsnprintf(NULL, 0, format, args)+1;
  char* buffer = new char[bufferSize];

  vsnprintf(buffer, bufferSize, format, args);

  if (Bluefruit.connected()) 
      bleuart.print(buffer);
  Serial.print(buffer);

  va_end(args);

  delete(buffer);
}

#else

void comms_init(short myid) {
//  BLE.begin();
}

#endif
