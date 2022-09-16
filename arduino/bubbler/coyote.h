uint8_t coyote_batterylevel = 0;
uint16_t coyote_maxPower = 0;
uint8_t coyote_powerStep = 0;
int coyote_powerA;
int coyote_powerB;
int coyote_ax = 0; int coyote_ay = 0; int coyote_az = 0;
int coyote_bx = 0; int coyote_by = 0; int coyote_bz = 0;
boolean coyote_connected = false;
#define start_powerA 150 // max is usually 2000
#define start_powerB 150 // max is usually 2000
