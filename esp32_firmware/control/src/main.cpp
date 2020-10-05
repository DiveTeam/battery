#include <Arduino.h>
#include <Wire.h>
#include "io.h"
#include "globals.h"
#include "wakeup.h"
#include "BLE.h"
#include "io.h"

#define LOW_LIMIT 33 //33% SOC
#define MEDIUM_LIMIT 66 //66% SOC
#define IDLE_TIMEOUT 1800000 // 30 minutes no load = turn off
#define KEEP_VIBRO_RUNNING_FOR 1500

BLE BT;

uint8_t sleep_is_set = 0;

void process_LED(){
  if(last_led_processed_at != millis_since_start){
    if(current_state.is_leaking){
      int8_t blinker = ((millis_since_start / 2) % 0x1FE) - 0xFF;
      uint8_t val = 0;
      if(blinker==0){
        leakage_blinks_made++;
      }
      if(blinker < 0){
        val = 0xFF - blinker;
      }else{
        val = blinker;
      }
      set_LED(val,0,0);
      if(leakage_blinks_made % 10 == 0){//vibro every 10th blink
        ledcWrite(3, 255);
      }else{
        ledcWrite(3, 0);
      }
    }else{
      if(millis_since_start > KEEP_VIBRO_RUNNING_FOR){
        ledcWrite(3, 0);//turn off vibro just in case
      }else{
        ledcWrite(3, 255);
      }
      leakage_blinks_made = 0;
      if(current_state.current > 0.3){ //charging with at least 0.3A current
        int8_t blinker = ((millis_since_start / 15) % 0x1FE) - 0xFF;
        uint8_t val = 0;
        if(blinker < 0){
          val = 0xFF - blinker;
        }else{
          val = blinker;
        }
        set_LED(0,0,val/2); //not too bright
      }else{
        if(current_state.soc <= LOW_LIMIT){ //hardcoded low limit to 33% SOC
          set_LED(0xFF,0,0);
        }else if(current_state.soc > LOW_LIMIT && current_state.soc <= MEDIUM_LIMIT){
          set_LED(0,0,0xFF);
        }else{
          set_LED(0,0xFF,0);
        }
      }
    }
    last_led_processed_at = millis_since_start;
  }
}

void update_current_state(){
  current_state.soc = gauge.getSOC();
  current_state.v1 = (float)(gauge.getCellVoltage(0) / 1000.0);
  current_state.v2 = (float)(gauge.getCellVoltage(1) / 1000.0);
  current_state.v3 = (float)(gauge.getCellVoltage(2) / 1000.0);
  current_state.current = (float)(gauge.getCurrent() / 1000.0);
  current_state.temperature = (float)(gauge.getTemperature());

  current_state.is_leaking = is_leaking() ? 1 : 0;
  Serial.print("V:");
  Serial.println(current_state.v1 + current_state.v2 + current_state.v3);
}

void gentle_gauge_poll(){
  if((millis_since_start - last_gauge_updated_at) > 100){ //get one characteristic per 50ms, will have ~300ms full sycle
    switch(last_gauge_updatec_characteristics){
      case 0:
        current_state.soc = gauge.getSOC();
        break;
      case 1:
        current_state.v1 = (float)(gauge.getCellVoltage(0) / 1000.0);
        break;
      case 2:
        current_state.v2 = (float)(gauge.getCellVoltage(1) / 1000.0);
        break;
      case 3:
        current_state.v3 = (float)(gauge.getCellVoltage(2) / 1000.0);
        break;
      case 4:
        current_state.current = (float)(gauge.getCurrent() / 1000.0);
        break;
      case 5:
        current_state.temperature = (float)(gauge.getTemperature());
        break;
      default:
        break;
    }
  
    last_gauge_updatec_characteristics++;
    if(last_gauge_updatec_characteristics > 5){
      last_gauge_updatec_characteristics = 0;
    }

    current_state.is_leaking = is_leaking() ? 1 : 0;
    last_gauge_updated_at = millis_since_start;
  }
}

void setup() {
  Serial.begin(115200);


  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  setup_led();
  setup_io();
  setup_bms();
  setup_eeprom();
  BT.begin("DiveTeam Battery");
  
  update_current_state();

  uint32_t log_position = get_log_position();
  BT.set_log_position(log_position);

  process_LED();
  turn_on();

  // reset_position_counter();
  // log_record record_to_read = { 0, 0, 0, 0, 0, 0.0, 0.0, 0.0 };
  // read_log_record(log_position, record_to_read);
}

void loop() {
  millis_since_start = millis();

  if(sleep_is_set == 0 && millis_since_start > (KEEP_VIBRO_RUNNING_FOR + 500)){ // allow turning off after ~0.5 seconds after full start
    esp_set_deep_sleep_wake_stub(&wake_stub);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 0);//wake up on button
    esp_sleep_enable_ext1_wakeup(0x800000000, ESP_EXT1_WAKEUP_ALL_LOW);//wake up on charger connection
    attachInterrupt(34, button_pressed_isr, FALLING);
    sleep_is_set = 1;
  }

  //it could have been freertos tasks, but I2C + BLE combination in tasks fucks everyting up. So NO TASKS HERE
  gentle_gauge_poll();

  if(current_state.current < -0.15 || current_state.current > 0){// found _some_ load (at least 150mA) or charger is on
    last_load_detected_at = millis_since_start;
  }

  if((millis_since_start - last_load_detected_at) > IDLE_TIMEOUT){
    wanna_turn_off = 1;
  }

  process_LED();
  
  uint32_t last_log_record_id = make_log_record();
  if(last_log_record_id > 0){
    BT.set_log_position(last_log_record_id);
  }

  if(wanna_turn_off){
    BT.end();
    turn_off();
  }

  if((millis_since_start - last_ble_notified_at) > 1000 && _BLEClientConnected){
    BT.notify_ble(current_state);
    last_ble_notified_at = millis_since_start;
  }

  if(intention_to_read_address >= 0){
    log_record record_to_read = { 0, 0, 0, 0, 0, 0.0, 0.0, 0.0 };
    read_log_record(intention_to_read_address, record_to_read);
    intention_to_read_address = -1;
    BT.notify_history(record_to_read);
  }
}
