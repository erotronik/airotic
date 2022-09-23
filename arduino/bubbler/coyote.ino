// DG-Lab estim box support.
// This file first has the methods that are implementation independent, followed by
// the Itsy Bitsy implementation, and then the ESP32 methods.

#ifdef ENABLE_BLE

// References:
// https://rezreal.github.io/coyote/web-bluetooth-example.html
// https://github.com/OpenDGLab/OpenDGLab-Connect/blob/master/src/services/DGLab.js

uint8_t COYOTE_SERVICE_UUID[] = {0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x0b, 0x18, 0x5a, 0x95};
uint8_t CONFIG_CHAR_UUID[] = {0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x07, 0x15, 0x5a, 0x95};
uint8_t POWER_CHAR_UUID[] = {0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x04, 0x15, 0x5a, 0x95};
uint8_t A_CHAR_UUID[] = {0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x06, 0x15, 0x5a, 0x95};
uint8_t B_CHAR_UUID[] = {0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x05, 0x15, 0x5a, 0x95};
uint8_t BATTERY_UUID[] = {0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x0a, 0x18, 0x5a, 0x95};
uint8_t BATTERY_CHAR_UUID[] = {0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x00, 0x15, 0x5a, 0x95};

int waveclocka = 0;
int wavemodea = 0;
int waveclockb = 0;
int wavemodeb = 0;
int cyclecount = 0;

TimerHandle_t coyoteTimer = nullptr;

#ifndef ESP32

BLEClientService coyoteService(COYOTE_SERVICE_UUID);
BLEClientCharacteristic configCharacteristic(CONFIG_CHAR_UUID);
BLEClientCharacteristic powerCharacteristic(POWER_CHAR_UUID);
BLEClientCharacteristic patternACharacteristic(A_CHAR_UUID);
BLEClientCharacteristic patternBCharacteristic(B_CHAR_UUID);
BLEClientService batteryService(BATTERY_UUID);
BLEClientCharacteristic batteryLevelCharacteristic(BATTERY_CHAR_UUID);

#else

NimBLEUUID COYOTE_SERVICE_BLEUUID(COYOTE_SERVICE_UUID, 16, false);
NimBLEUUID CONFIG_CHAR_BLEUUID(CONFIG_CHAR_UUID, 16, false);
NimBLEUUID POWER_CHAR_BLEUUID(POWER_CHAR_UUID, 16, false);
NimBLEUUID A_CHAR_BLEUUID(A_CHAR_UUID, 16, false);
NimBLEUUID B_CHAR_BLEUUID(B_CHAR_UUID, 16, false);
NimBLEUUID BATTERY_SERVICE_BLEUUID(BATTERY_UUID, 16, false);
NimBLEUUID BATTERY_CHAR_BLEUUID(BATTERY_CHAR_UUID, 16, false);

NimBLERemoteService* coyoteService;
NimBLERemoteCharacteristic* configCharacteristic;
NimBLERemoteCharacteristic* powerCharacteristic;
NimBLERemoteCharacteristic* patternACharacteristic;
NimBLERemoteCharacteristic* patternBCharacteristic;
NimBLERemoteService* batteryService;
NimBLERemoteCharacteristic* batteryLevelCharacteristic;

#endif

// called when we get a breath (true) or release (false, ignore it)
void coyote_cb(boolean b) {
  if (!coyote_connected) return;
  if (b) { 
    if (cyclecount == 0) Serial.println("Coyote start");
    cyclecount +=2; // for a and b
  }
}

void coyote_wave_ramp(int *waveclock, int *wavemode, int *ax, int *ay, int *az) {
  //
  // wavemode 0 - don't send anything (off)
  // wavemode 1 - like the 'breathe' mode (1100mS cycle)
  //
  *ax = 0; *ay = 0; *az = 0;
  if (cyclecount == 0) return;
  if (*wavemode == 1) {
    //instead of settting up an array like this, let's write it as a formula
    //Int8Array(3) [33, 1, 0]  0: 1 9 0
    //Int8Array(3) [33, 1, 2]  1: 1 9 4
    //Int8Array(3) [33, 1, 4]  2: 1 9 8
    //Int8Array(3) [33, 1, 6]  3: 1 9 12
    //Int8Array(3) [33, 1, 8]  4: 1 9 16
    //Int8Array(3) [33, 1, 10] 5: 1 9 20
    //Int8Array(3) [33, 1, 10] 6: 1 9 20
    //Int8Array(3) [33, 1, 10] 7: 1 9 20
    //Int8Array(3) [0, 0, 0]   8: 0 0 0
    //Int8Array(3) [0, 0, 0]   9: 0 0 0
    //Int8Array(3) [0, 0, 0]  10: 0 0 0
    if (*waveclock < 8) {
      *ax = 1;
      *ay = 9;
      *az = *waveclock * 4;
      if (*az > 20) *az = 20;
    }
    (*waveclock)++;
    if (*waveclock > (7+3)) { 
      *waveclock = 0;
      cyclecount--;
      if (cyclecount == 0) {
          Serial.println("Coyote stop");
      }
    }   
  }
}

// This is called every 100mS to provide the coyote box with what to do next
//
void coyote_timer_callback(TimerHandle_t xTimerID)
{
  uint8_t buf[4];

  if (!coyote_connected) return;
  if (coyote_powerA > 0 && wavemodea != 0) {
    coyote_wave_ramp(&waveclocka, &wavemodea, &coyote_ax, &coyote_ay, &coyote_az);
    coyote_encode_pattern(buf, coyote_ax, coyote_ay, coyote_az);
#ifdef ESP32
    if ( !patternACharacteristic->writeValue(buf, 3) )
      Serial.println("Failed to write pattern A");
#else 
    while (!patternACharacteristic.write(buf, 3)) {};
#endif
  }
  if (coyote_powerB > 0 && wavemodeb != 0) {
    coyote_wave_ramp(&waveclockb, &wavemodeb, &coyote_bx, &coyote_by, &coyote_bz);
    coyote_encode_pattern(buf, coyote_bx, coyote_by, coyote_bz);
#ifdef ESP32
    if ( ! patternBCharacteristic->writeValue(buf, 3) ) {
      Serial.println("Failed to write pattern B");
    }
#else
    while (!patternBCharacteristic.write(buf, 3)) {} ;
#endif
  }
  // this would reset the power back to stop you changing it on the rocker switches
  // if (selectedPowerA !== devicePowerA || selectedPowerB !== devicePowerB)
  //   power.writeValue(encodePower(selectedPowerA, selectedPowerB));
}

void coyote_parse_power(const uint8_t *buf) {
  // notify/write: 3 bytes: flipFirstAndThirdByte(zero(2) ~ uint(11).as("powerLevelB") ~uint(11).as("powerLevelA")
  coyote_powerA = (buf[2] * 256 + buf[1]) >> 3;
  coyote_powerB = (buf[1] * 256 + buf[0]) & 0b0000011111111111;
  Serial.printf("coyote power A=%d B=%d\n", coyote_powerA, coyote_powerB);
}

void coyote_encode_power(uint8_t *buf, int xpowerA, int xpowerB) {
  // notify/write: 3 bytes: flipFirstAndThirdByte(zero(2) ~ uint(11).as("powerLevelB") ~uint(11).as("powerLevelA")
  buf[2] = (xpowerA & 0b11111100000) >> 5;
  buf[1] = (xpowerA & 0b11111) << 3 | (xpowerB & 0b11100000000) >> 8;
  buf[0] = xpowerB & 0xff;
  Serial.printf("coyote power A=%d B=%d\n", xpowerA, xpowerB);
}

void coyote_encode_pattern(uint8_t *buf, int ax, int ay, int  az) {
  // flipFirstAndThirdByte(zero(4) ~ uint(5).as("az") ~ uint(10).as("ay") ~ uint(5).as("ax"))
  buf[2] = (az & 0b11111) >> 1;
  buf[1] = ((az & 0b1) * 128) | ((ay & 0b1111111000) >> 3);
  buf[0] = (ax & 0b11111) | ((ay & 0b111) << 5);
  //Serial.printf("encode pattern %02x %02x %02x ax=%d ay=%d az=%d\n", buf[2],buf[1],buf[0],ax,ay,az);
}

void coyote_parse_pattern(const uint8_t *buf, int *ax, int *ay, int *az) {
  // flipFirstAndThirdByte(zero(4) ~ uint(5).as("az") ~ uint(10).as("ay") ~ uint(5).as("ax"))
  *az = ((buf[2] * 256 + buf[1]) & 0b0000111110000000) >> 7;
  *ay = ((buf[2] * 256 * 256 + buf[1] * 256 + buf[0]) & 0b000000000111111111100000) >> 5;
  *ax = (buf[0]) & 0b11111;
  //Serial.printf("parse pattern %02x %02x %02x ax=%d ay=%d az=%d\n", buf[2],buf[1],buf[0],*ax,*ay,*az);
}

#ifndef ESP32

void coyote_setup(void) {
#if 0
  uint8_t x[4];
  x[0] = 33; x[1] = 1; x[2] = 10;
  coyote_parse_pattern(x, &coyote_ax, &coyote_ay, &coyote_az);
  coyote_encode_pattern(x, coyote_ax, coyote_ay, coyote_az);
  x[0] = 33; x[1] = 1; x[2] = 8;
  coyote_parse_pattern(x, &coyote_ax, &coyote_ay, &coyote_az);
  x[0] = 33; x[1] = 1; x[2] = 0;
  coyote_parse_pattern(x, &coyote_ax, &coyote_ay, &coyote_az);
#endif

  batteryService.begin();
  batteryLevelCharacteristic.begin();
  coyoteService.begin();
  configCharacteristic.begin();
  powerCharacteristic.begin();
  patternACharacteristic.begin();
  patternBCharacteristic.begin();
}

void central_disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.println("coyote disconnected");
  coyote_connected = false;
  xTimerStop(coyoteTimer, 0);
}

void coyote_batterylevel_callback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
  coyote_batterylevel = data[0];
  Serial.printf("coyote batterycb: %d\n", coyote_batterylevel);
}

void coyote_power_callback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len) {
  coyote_parse_power(data);
}

void connectionfailed(String x,uint16_t conn_handle) {
  Serial.println(x);
  Bluefruit.disconnect(conn_handle);
  return;
}

void central_connect_callback(uint16_t conn_handle) {
  uint8_t buf[20];

  if (!batteryService.discover(conn_handle)) return connectionfailed("e1",conn_handle);
  if (!batteryLevelCharacteristic.discover()) return connectionfailed("e2",conn_handle);

  while (!batteryLevelCharacteristic.read(buf, 1)) {};
  coyote_batterylevel = buf[0];
  Serial.printf("coyote battery %d\n", coyote_batterylevel );
  batteryLevelCharacteristic.setNotifyCallback(coyote_batterylevel_callback);
  batteryLevelCharacteristic.enableNotify();

  if (!coyoteService.discover(conn_handle)) return connectionfailed("e3",conn_handle);
  if (!configCharacteristic.discover()) return connectionfailed("e4",conn_handle);

  while (!configCharacteristic.read(buf, 3)) {};
  // read: 3 bytes: flipFirstAndThirdByte(skip(5) ~ uint(11).as("maxPower") ~ uint8.as("powerStep"))
  coyote_maxPower = (buf[2] & 0xf) * 256 + buf[1];
  coyote_powerStep = buf[0];
  Serial.printf("coyote maxPower: %d\n", coyote_maxPower);
  Serial.printf("coyote powerStep: %d\n", coyote_powerStep);

  if (!powerCharacteristic.discover()) return connectionfailed("e5",conn_handle);
  
  while (!powerCharacteristic.read(buf, 3)) {};
  coyote_parse_power(buf);
  powerCharacteristic.setNotifyCallback(coyote_power_callback);
  powerCharacteristic.enableNotify();

  if (!patternACharacteristic.discover()) return connectionfailed("e6",conn_handle);
  if (!patternBCharacteristic.discover()) return connectionfailed("e7",conn_handle);

  while (!patternACharacteristic.read(buf, 3)) {};
  coyote_parse_pattern(buf, &coyote_ax, &coyote_ay, &coyote_az);
  while (!patternBCharacteristic.read(buf, 3)) {};
  coyote_parse_pattern(buf, &coyote_bx, &coyote_by, &coyote_bz);

  coyote_encode_power(buf, start_powerA, start_powerB);
  while (powerCharacteristic.write(buf, 3)) {};
  wavemodea = 1;
  wavemodeb = 1;

  coyote_connected = true;

  cyclecount = 6; // 3 a and 3 b
  if (!coyoteTimer)
    coyoteTimer = xTimerCreate(nullptr, pdMS_TO_TICKS(100), true, nullptr, coyote_timer_callback); 
  xTimerStart(coyoteTimer, 0);

  Serial.println("coyote connected!");
}

#endif /* not ESP32 */

#ifdef ESP32


NimBLEClient* bleClient = nullptr;

class BubblerClientCallback : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pclient) {
    Serial.println("Client onConnect");
  }

  void onDisconnect(NimBLEClient* pclient) {
    coyote_connected = false;
    xTimerStop(coyoteTimer, 0);
    Serial.println("Client onDisconnect");
  }
};

void coyote_setup(void) {
  // nothing to do for the ESP.
}

bool getService(NimBLERemoteService*& service, NimBLEUUID uuid) {
  Serial.printf("Getting service\n");
  service = bleClient->getService(uuid);
  if (service == nullptr) {
    Serial.print("Failed to find service UUID: ");
    Serial.println(uuid.toString().c_str());
    return false;
  }
  return true;
}

bool getCharacteristic(NimBLERemoteService* service, NimBLERemoteCharacteristic*& c, NimBLEUUID uuid, notify_callback notifyCallback = nullptr) {
  Serial.printf("Getting characteristic\n");
  c = service->getCharacteristic(uuid);
  if (c == nullptr) {
    Serial.print("Failed to find characteristic UUID: ");
    Serial.println(uuid.toString().c_str());
    return false;
  }

  if ( !notifyCallback )
    return true;

  // we want notifications
  if( c->canNotify() && c->subscribe(true, notifyCallback) )
    return true;
  else {
    Serial.print("Failed to register for notifications for characteristic UUID: ");
    Serial.println(uuid.toString().c_str());
    return false;
  }
}

void coyote_batterylevel_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify) {
  coyote_batterylevel = data[0];
  Serial.printf("coyote batterycb: %d\n", coyote_batterylevel);
}

void coyote_power_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify) {
  if ( length >=3 )
    coyote_parse_power(data);
  else
    Serial.println("Power callback with incorrect length");
}

bool connect_to_coyote(NimBLEAdvertisedDevice* coyote_device) {

  if ( coyote_connected )
    return false;

  if ( ! bleClient ) {
    bleClient = NimBLEDevice::createClient();
    bleClient->setClientCallbacks(new BubblerClientCallback());
  }

  Serial.printf("Will try to connect to Coyote at %s\n", coyote_device->getAddress().toString().c_str());
  if ( !bleClient->connect(coyote_device) ) {
    Serial.println("Connection failed");
    return false;
  }
  Serial.println("Connection established");

  bool res = true;
  res &= getService(coyoteService, COYOTE_SERVICE_BLEUUID);
  res &= getService(batteryService, BATTERY_SERVICE_BLEUUID);

  if ( res == false ) {
    Serial.println("Missing service");
    bleClient->disconnect();
    return false;
  }

  res &= getCharacteristic(batteryService, batteryLevelCharacteristic, BATTERY_CHAR_BLEUUID, coyote_batterylevel_callback);
  res &= getCharacteristic(coyoteService, configCharacteristic, CONFIG_CHAR_BLEUUID);
  res &= getCharacteristic(coyoteService, powerCharacteristic, POWER_CHAR_BLEUUID, coyote_power_callback);
  res &= getCharacteristic(coyoteService, patternACharacteristic, A_CHAR_BLEUUID);
  res &= getCharacteristic(coyoteService, patternBCharacteristic, B_CHAR_BLEUUID);

  if ( res == false ) {
    Serial.println("Missing characteristic");
    bleClient->disconnect();
    return false;
  }

  Serial.println("Found services and characteristics");

  coyote_connected = true;

  coyote_batterylevel = batteryLevelCharacteristic->readUInt8();
  Serial.printf("Battery level: %u\n", coyote_batterylevel);

  auto configData = configCharacteristic->readValue();
  coyote_maxPower = (configData[2] & 0xf) * 256 + configData[1];
  coyote_powerStep = configData[0];
  Serial.printf("coyote maxPower: %d\n", coyote_maxPower);
  Serial.printf("coyote powerStep: %d\n", coyote_powerStep);

  auto powerData = powerCharacteristic->readValue();
  if ( powerData.size() >= 3 )
    coyote_parse_power(powerData.begin());

  auto patternAData = patternACharacteristic->readValue();
  auto patternBData = patternBCharacteristic->readValue();
  if ( patternAData.size() < 3 || patternBData.size() < 3 ) {
    Serial.println("No pattern data?");
  }
  coyote_parse_pattern(patternAData.begin(), &coyote_ax, &coyote_ay, &coyote_az);
  coyote_parse_pattern(patternBData.begin(), &coyote_bx, &coyote_by, &coyote_bz);

  uint8_t buf[5];
  coyote_encode_power(buf, start_powerA, start_powerB);
  if ( !powerCharacteristic->writeValue(buf, 3) )
    Serial.println("Failed to write powerCharacteristic");

  wavemodea = 1;
  wavemodeb = 1;

  cyclecount = 6; // 3 a and 3 b
  if (!coyoteTimer)
    coyoteTimer = xTimerCreate(nullptr, pdMS_TO_TICKS(100), true, nullptr, coyote_timer_callback);
  xTimerStart(coyoteTimer, 0);


  Serial.println("coyote connected");

  return true;
}

#endif /* ESP32 */
#endif /* ENABLE_BLE */
