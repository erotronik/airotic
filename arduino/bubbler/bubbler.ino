// Bluetooth Bubbler Bottle
//
// Bottle lights a (configurable) "at rest" colour. On every detected
// breath it changes to a (configurable) "breath" colour then fades
// back to "at rest".
//
// Bottles detect other bottles in range and when close enough they will
// sync their lights to match colours. They do this by looking at the
// bluetooth advertising data and do not connect to each other.
//
// You can connect to the Bottle by bluetooth to configure it, you also
// get a notification of breath and release so you can "make other things
// happen"
//
// Hardware:
//
// ItsyBitsy nRF52840 Express
//
// BMP280 on SDL/SCA/3v/0v, two WS2812b (neopixels) chained on pin 5/+V/0v
//
// Use the Adafruit controller app "Controller"
// Push Control pad 1 - then use color picker for "at rest" colour
// Push Control pad 2 - then use color picker for "bubble" colour
// Push Control pad 3 - to reset to more sensible defaults
// Push Control pad 4 - restart device so we advertise correct colours and name
// Use UART mode and type "!S1" to set device to ID 1 (1-9 allowed)
//
// This version will also look for DG-Labs Coyote bluetooth powerboxes. If it
// finds one it will connect to it and send 5 'breath' waves to both A and B
// channels at level '150' (defined in coyote.h, usually 2000 max). Then every breath
// sends a 'breath' wave to channels A and B. You can alter the level up and down
// using the rocker switches on the box, these are forgotten when you disconnect.  Holding
// a rocker switch will reset the channel to zero and you'll need to disconnect or restart
// the bottle to get back to the default.

// rssi average below which we merge the LEDS of seen bottles
// not configurable as kind of needs to be the same on each bottle to work
#define DISTANCE_TO_SYNC_LEDS 64

#include <FastLED.h>
#include <bluefruit.h>

#define MAX_BOTTLES 16
typedef struct {
  CHSV colorTarget;
  CHSV colorStart;
  int rssi_av;
  long rssi_last;
  short state; // 0 away, 1 close
  boolean changed_state;
} bottle_t;
bottle_t seen_bottles[MAX_BOTTLES];

#define NUM_LEDS 2
CRGB leds[NUM_LEDS];

CHSV colorTargetDefault = CHSV(160, 255, 255); // blue is 160, green is 96
CHSV colorStartDefault = CHSV(224, 255, 255); // purple is 192, red is 0
CHSV colorTarget = colorTargetDefault;
CHSV colorMyTarget = colorTargetDefault;
CHSV colorStart = colorStartDefault;
CHSV colorWas;
CHSV colorMyStart = colorStart;
CHSV colorCurrent = colorTarget;
uint8_t fadespeed = 64;
short bottle_number = 1;
int avg;
short blow_state = 0;
int fade = 0;
long xm = millis();
long y = 0;
int nearest_bottle_percent = 100;

// set this via bluetooth, 1==bluetooth send the bmp sensor read and average
short debug_mode = 0;

void leds_setup() {
  // "Boot" mode
  FastLED.addLeds<NEOPIXEL, 5>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  leds[0] = CRGB::White;
  leds[1] = CRGB::White;
  FastLED.show();
}

void leds_startingstate() {
  colorCurrent = colorMyTarget;
  fill_solid( leds, NUM_LEDS, colorCurrent );
  FastLED.show();
}

void leds_do_fade() {
  // If we're not 'at rest' colour then fade towards it
  if ( colorCurrent.h != colorTarget.h ) {
    fade = min(fade + fadespeed, 255);
    colorCurrent = blend(colorWas, colorTarget, fade, SHORTEST_HUES);
    fill_solid( leds, NUM_LEDS, colorCurrent );
    FastLED.show();
  }
}

void check_breath() {
  //
  // Read the sensor and get an average, detect the start of a blow/suck and the end of a blow/suck
  // the detection of the end isn't accurate, but we do a fade so you won't notice.
  //
  // moving average was by trial and error, let's do 60% old, 40% new
  // blow threshold was by trial and error, 50 seemed to work quite well
  // 20210729 move avg to end, 80% old 20% new and threshosd 15
  //
  #define avthres 15
  int pread = airsensor_read();
  if (blow_state == 0 && pread < (avg - avthres)) {
    blow_state = 1;
    fade = 0;
    // fade, but slow while the breath is happening
    comms_send_breath(true);
    fadespeed = 4;
    colorCurrent = colorStart;
    colorWas = colorStart;
  } else if (blow_state == 1 && pread > avg) {
    blow_state = 0;
    comms_send_breath(false);
    // fade back faster now the breath has 'stopped'
    fadespeed = 16;
  }
  if (debug_mode & 1) comms_uart_send_graph(pread, avg);
  //Serial.printf("%d,%d,%d\n",pread,avg,avg-avthres);
  avg = (avg * 8 + pread * 2) / 10;

}

void debug_checkcodespeed() {
  //
  // just for debugging: if something in the code is eating all the time we might need to be more careful with the delays
  //
  y++;
  if (millis() > xm + 1000) {
    xm = millis();
    if (y < 9) Serial.printf("SLOWTICK %d\n", y);
    y = 0;
  }
  if (millis() / 100 % 30 == 0) {
    for (short i = 0; i < MAX_BOTTLES; i++) {
      if (seen_bottles[i].rssi_av < 128) {
        Serial.printf("Bottle %d : %d\n", i, seen_bottles[i].rssi_av);
      }
    }
  }
}

void look_for_close_bottles() {
  //
  // Any other Bottles close?  "Merge" colours if 2, Fixed colour if 3
  //
  short n = 0;
  int closestd = 128;
  if (debug_mode & 2) comms_uart_send_bottles();

  for (short i = 0; i < MAX_BOTTLES; i++) {
    if (seen_bottles[i].state != 0) {
      if (seen_bottles[i].rssi_av < closestd) closestd = seen_bottles[i].rssi_av;
    }
  }
  nearest_bottle_percent = min(0,closestd-28);

  for (short i = 0; i < MAX_BOTTLES; i++) {
    if (seen_bottles[i].changed_state) {
      n = 1;
      Serial.printf("State Change Bottle %d : %d\n", i, seen_bottles[i].rssi_av);      
      seen_bottles[i].changed_state = false;
    }
  }
  if (n == 0) return;

  // n = 1 if any bottles have changed state, figure out how many are close

  n = 0;
  for (short i = 0; i < MAX_BOTTLES; i++) {
    if (seen_bottles[i].state != 0) {
      n++;
      colorTarget = blend(colorMyTarget, seen_bottles[i].colorTarget, 128, SHORTEST_HUES);
      colorStart = blend(colorMyStart, seen_bottles[i].colorStart, 128, LONGEST_HUES);          
    }
  }
  Serial.printf("Bottles close: %d\n",n);

  if (n == 0) {
    colorTarget = colorMyTarget;
    colorStart = colorMyStart;
  } else if (n > 1) {
      //
      // 2 or more bottles? blend is no use so let's just use a fixed default
      //
      colorTarget = CHSV(255, 255, 255);
      colorStart = CHSV(42, 255, 255);
  } // otherwise n==1 use the one we found above.
}

void bottle_setup() {
  // MAX_BOTTLES green bottles, sitting on a wall...
  for (short i = 0; i < MAX_BOTTLES; i++) {
    seen_bottles[i].rssi_av = 128;
    seen_bottles[i].changed_state = false;
  }
}

// Main Entry Points are Below

void setup() {
  Serial.begin(115200);
  bottle_setup();
  leds_setup();
  delay(1500);
  airsensor_setup();
  avg = airsensor_read();
  storage_setup();
  leds_startingstate();
  comms_init(bottle_number);
  Serial.printf("I am bottle number %d\n", bottle_number);
}

void loop() {
  comms_uart_colorpicker();
  debug_checkcodespeed();
  check_breath();
  comms_check_distance(DISTANCE_TO_SYNC_LEDS);
  look_for_close_bottles();
  leds_do_fade();
  delay(100);
}
