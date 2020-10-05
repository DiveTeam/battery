#include "io.h"
#include "globals.h"
#include "soc/ledc_reg.h"
#include "soc/ledc_struct.h"
// #include "max1720x.h"
// #include <Arduino.h>
// #include <Wire.h>
#include <extEEPROM.h>


#define VIBRO_PIN 5

//this is just to make it look pretty and have some spare bytes. try not exceed 29-30 bytes to avoid buffer tearing (Arduino cant send more thatn 32 bytes at once)
#define LOG_RECORD_SIZE 32
#define LOG_FIRST_RECORD_OFFSET 256 //let's leave first 256 bytes (page) for other variables
#define LOGS_FIT_TO_EEPROM 7800
#define EEPROM_ADDR_POSITION_COUNTER 0


extEEPROM myEEPROM(kbits_2048, 1, 256); //single EEPROM, 256 bytes page

void setup_led(){
  ledcSetup(0, 5000, 8); //setting up PWM (3 channels) at 5Hz frequency with 8-bit resolution
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcAttachPin(26, 0);//RED
  ledcAttachPin(25, 1);//GREEN
  ledcAttachPin(27, 2);//BLUE
  ledcWrite(0, 255);
  ledcWrite(1, 255);
  ledcWrite(2, 255);
}


void IRAM_ATTR button_pressed_isr(){
  wanna_turn_off = 1;
  pinMode(VIBRO_PIN, OUTPUT);
  digitalWrite(VIBRO_PIN, 1);
  //avoid ANYTHING that could potentially be not in RAM. Any functions are bad!
}

void set_LED(uint8_t R, uint8_t G, uint8_t B){
  ledcWrite(0, R);
  ledcWrite(1, G);
  ledcWrite(2, B);
}

void setup_io(){
  //CHARGER_ON -> IO35. aka RTC_GPIO_5. has external 100k pullup to VDD
  //BUTTON -> IO34. aka RTC_GPIO_4. has external 100k pullup to VDD
  //ALRT1 -> IO39 (SVN). aka RTC_GPIO_3. has external 100k pullup to VDD. Alert from BMS
  //LEAKAGE_IN -> IO36 (SVP). aka RTC_GPIO0. has external 100k pullup to VDD
  //OUT_CONTROL -> IO19
  //LEAKAGE_OUT -> IO18
  //VIBRO -> IO5
  // pinMode(VIBRO_PIN, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(34, INPUT);
  pinMode(35, INPUT);
  pinMode(36, INPUT);
  pinMode(39, INPUT);

  ledcSetup(3, 5000, 8); //setting up PWM
  ledcAttachPin(VIBRO_PIN, 3);//VIBRO
  ledcWrite(3, 255);
}

void setup_eeprom(){
  byte i2cStat = myEEPROM.begin(extEEPROM::twiClock400kHz);
  if ( i2cStat != 0 ) {
    Serial.println("Unable to start I2C communication");
  }
}

void setup_bms(){
  gauge.writeReg(0x00, 0x0000);		//clear status register
}

uint32_t get_eeprom_offset(uint32_t log_id){
  uint32_t result = ((log_id % LOGS_FIT_TO_EEPROM) * LOG_RECORD_SIZE) + LOG_FIRST_RECORD_OFFSET;
  return result;
}

uint32_t get_log_position(){
  byte readData[4];
  byte i2cStat = myEEPROM.read(EEPROM_ADDR_POSITION_COUNTER, readData, 4);

  if ( i2cStat != 0 ) {
    if ( i2cStat == EEPROM_ADDR_ERR) {
      Serial.println("Error reading data - bad address");
    }
    else {
      Serial.println("Error reading data - I2C error");
    }
  }else{
    uint32_t log_position =  ((uint32_t)readData[3] << 24) + ((uint32_t)readData[2] << 16) + ((uint32_t)readData[1] << 8) + (uint32_t)readData[0];
    return log_position;
  }
  return 0;
}

void IRAM_ATTR read_log_record(uint32_t id, log_record &record_to_read){
  // Serial.print("Offset: ");
  // Serial.println(get_eeprom_offset(id));

  byte* p = ( byte*)( void*)&record_to_read;
  myEEPROM.read(get_eeprom_offset(id), p, sizeof(log_record));
  // Serial.print("Record ID: ");
  // Serial.println(record_to_read.record_id);
}

uint32_t make_log_record(){
  //once a minute + at the start
  if(millis_since_start - last_record_saved_at >= 60000 || last_record_saved_at == 0){
    Serial.print("Making log record ");

    uint32_t log_position = get_log_position();
    current_state.record_id = log_position;
    byte* p = ( byte*)( void*)&current_state;
    Serial.print(log_position + 1);
    Serial.print(" with offset ");
    Serial.println(get_eeprom_offset(log_position + 1));

    // Serial.print("Writing DATA: ");
    // for(uint8_t i=0;i<26;i++){
    //   Serial.println(p[i], HEX);
    // }


    byte i2cStat = myEEPROM.write(get_eeprom_offset(log_position + 1), p, sizeof(current_state));
    if ( i2cStat == 0 ){
      delay(10); //10 ms delay to complete write operation.

      // Serial.print("Reading DATA: ");
      // for(uint8_t i=0;i<26;i++){
      //   Serial.println(myEEPROM.read(get_eeprom_offset(log_position)+i), HEX);
      // }

      log_position++;
      byte new_log_position[4];
      new_log_position[0] = log_position & 0xFF;
      new_log_position[1] = (log_position >> 8) & 0xFF;
      new_log_position[2] = (log_position >> 16) & 0xFF;
      new_log_position[3] = (log_position >> 24) & 0xFF;
      i2cStat = myEEPROM.write(EEPROM_ADDR_POSITION_COUNTER, new_log_position, sizeof(new_log_position)); //zero EEPROM address will store position
      // Serial.print("Updating log position to ");
      // Serial.println(log_position);
    }
    last_record_saved_at = millis_since_start;
    return log_position;
  }
  return 0;
}


void turn_on(){
  Serial.println("Turning on");
  while(!digitalRead(34)){}; //skip button pressed
  digitalWrite(19, 1);//turn on output transistor
  digitalWrite(18, 1);//leakage control oN
  
}


void notify_turning_off(){
  for(uint8_t i=0; i<5; i++){
    for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){   
      ledcWrite(3, dutyCycle);
      delay(1);
    }

    for(int dutyCycle = 255; dutyCycle >=0; dutyCycle--){   
      ledcWrite(3, dutyCycle);
      delay(1);
    }
  }
}

void turn_off(){
  Serial.println("Turning off");
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);
  digitalWrite(19, 0);//turn off output transistor
  while(!digitalRead(34)){}; //skip button pressed
  notify_turning_off();
  wanna_turn_off=0;
  // delay(1000);
  digitalWrite(18, 0);//leakage control off
  delay(500);//to let BT turnoff settle down
  esp_deep_sleep_start();
}


bool is_leaking(){
  return(!digitalRead(36));
}

void reset_position_counter(){
  byte zero_array[4]={0,0,0,0}; //reset log counter to 0
  myEEPROM.write(EEPROM_ADDR_POSITION_COUNTER, zero_array, sizeof(zero_array));
}