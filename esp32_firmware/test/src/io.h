#ifndef io_h
#define io_h

#include <Arduino.h>

//JTAG connection (FT232H):
//AC1->reset
//AD0->IO13 (FTDI => TCK)
//AD1->IO12 (FTDI => TDI)
//AD2->IO15 (FTDI => TDO)
//AD3->IO14 (FTDI => TMS)

//RED -> IO26
//GREEN -> IO25
//BLUE -> IO27


void setup_led();

void setup_io();

void save_bms_config();//WARNING: BMS memery can be written only 7 (SEVEN) times. Be wise, don't waste
  
void bms_initial_setup(); //only call once

void setup_bms();

void setup_eeprom();

void turn_on();

void turn_off();

void set_LED(uint8_t R, uint8_t G, uint8_t B);

bool check_eeprom();

bool is_bms_config_magic_mark_stored();

void make_bms_config_magic_mark();

void reset_position_counter();

void check_bms();

#endif
