BLEUart bleuart;

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

  comms_start_adv();
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
  Serial.print("Connected to ");
  Serial.println(central_name);

  btAndSerialPrintLn(strprintf("Connected to %s", central_name));
}

void comms_send_response(char const *response) {
  Serial.printf("Send Response: %s\n", response);
  bleuart.write(response, strlen(response)*sizeof(char));
}

void comms_send_breath(boolean b) {
  if (b) {
    btAndSerialPrintLn("*B");
  } else {
    btAndSerialPrintLn("*R");
  }
}

void comms_uart_send_bottles() {
  if (millis() / 100 % 5 != 0) return;

  for (short i = 0; i < MAX_BOTTLES; i++) {
     if (seen_bottles[i].rssi_av<128) {
       btAndSerialPrintLn(strprintf("%d\r\n",seen_bottles[i].rssi_av));
     }
  }
}

boolean comms_uart_send_graph(int pread, int avg) {
  btAndSerialPrintLn(strprintf("%d,%d\r\n",pread,avg));
  
  return true;
#if 0
  Serial.print(pread);
  Serial.print(",");
  Serial.print(avg);
  Serial.print(",");
  Serial.println(blow_state * 100 + 100800);
#endif
}

short last_button = 0;

void comms_uart_colorpicker(void) {
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
  
  if (!Bluefruit.connected() || !bleuart.notifyEnabled() ) {
    return;
  }
  
  int command = bleuart.read();
  
  if (command != '!') {
    return;
  }

  command = bleuart.read();

  // Introduce
  if (command == 'H') {
      btAndSerialPrintLn(strprintf("Hello I am bottle %d", bottle_number));
      return;
  } 

  // Set color
  if (command == 'C') {
    uint8_t r = bleuart.read();
    uint8_t g = bleuart.read();
    uint8_t b = bleuart.read();
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
    return;
  }

  if (command == 'B') {
    command = bleuart.read();

    // Reboot bottle
    if (command == '4') {
      NVIC_SystemReset();
    }
    last_button = command - '0';

    // Reset colors to defaults
    if (last_button == 3) {
      colorTarget = colorTargetDefault;
      colorStart = colorStartDefault;
      storage_write();
      comms_send_response("ok");
    }
    return;
  }

  // Set bottle number
  if (command == 'S') {
    command = bleuart.read();
    bottle_number = (command - '0')*10;
    
    command = bleuart.read();
    bottle_number += (command - '0');
     
    if (bottle_number < MAX_BOTTLES) {
      storage_write();
      comms_send_response("ok, set, rebooting");
      delay(100);
      NVIC_SystemReset();          
    } else {
      comms_send_response("bad number");
    }
    return;
  }

  // Set debug mode
  if (command == 'D') {
    command = bleuart.read();
    debug_mode = command - '0';
    comms_send_response("ok");
    return;
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
      Serial.printf("Bottle %d has departed\n", i);
    }
    if (seen_bottles[i].rssi_av <= distance && seen_bottles[i].state == 0) {
      Serial.printf("Bottle %d is close %d\n", i, seen_bottles[i].rssi_av);
      seen_bottles[i].state = 1;
      seen_bottles[i].changed_state = true;
    }
    if (seen_bottles[i].rssi_av >= (distance + 6) && seen_bottles[i].state == 1) {
      Serial.printf("Bottle %d is away %d\n", i, seen_bottles[i].rssi_av);
      seen_bottles[i].state = 0;
      seen_bottles[i].changed_state = true;
    }
  }
}

void scan_callback(ble_gap_evt_adv_report_t *report)
{
  uint8_t buffer[32];
  uint8_t len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buffer, sizeof(buffer));

  if (len > 2 && buffer[1] == our_fake_company_id1 && buffer[0] == our_fake_company_id0) {
    //
    // Found a Bottle
    //
    short bottleno = buffer[2] & 0xf;
    if (bottleno&1 == 1) {  // we only look at odd id bottles
      if (seen_bottles[bottleno].rssi_av == 128) {
        Serial.printf("How do you do fellow bottle #%d (%d)\n", bottleno, 0 - report->rssi);
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

void btAndSerialPrintLn(const char* format) {
  if (Bluefruit.connected()) {
    bleuart.println(format);
  }
  
  Serial.println(format);
}

char* strprintf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  
  int bufferSize = vsnprintf(NULL, 0, format, args);
  bufferSize++;  // safe byte for \0
  
  char* buffer = new char[bufferSize];

  vsnprintf(buffer, bufferSize, format, args);

  va_end(args);

  return buffer;
}
