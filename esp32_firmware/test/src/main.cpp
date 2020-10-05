#include <Arduino.h>
#include <Wire.h>
#include "test.h"
#include "io.h"
#include "io.h"

void setup() {
  Serial.begin(115200);

  setup_led();
  setup_io();
  setup_eeprom();
    
  reset_position_counter();

  check_leds();
  check_io();
  check_vibro();
  check_leakage();
  bool is_eeprom_ok = check_eeprom();
  // digitalWrite(19, 0);
  digitalWrite(19, 1);//turn on output (to avoid overheating internal FET diode)
  
  if(is_eeprom_ok){
    setup_bms();
    if(is_bms_config_magic_mark_stored()){
      Serial.println("BMS config has already been stored, NOT rewriting");
      
    }else{
      Serial.println("BMS config has not been saved yet, setting UP BMS");
      bms_initial_setup();
      make_bms_config_magic_mark();
    }
    check_bms();
  }else{
    Serial.println("Will not move on until EEPROM issues are sorted out");
  }
}


void loop() {

}

