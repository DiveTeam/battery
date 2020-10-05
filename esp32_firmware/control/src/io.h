#ifndef io_h
#define io_h

#include <Arduino.h>
#include "globals.h"

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

void setup_bms();

void setup_eeprom();

void turn_on();

void turn_off();

void set_LED(uint8_t R, uint8_t G, uint8_t B);

bool is_leaking();

void IRAM_ATTR button_pressed_isr();

uint32_t get_eeprom_offset(uint32_t log_id);

uint32_t make_log_record();

void IRAM_ATTR read_log_record(uint32_t id, log_record &record_to_read);

uint32_t get_log_position();

void reset_position_counter();

#endif
