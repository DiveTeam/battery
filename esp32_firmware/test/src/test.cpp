#include "test.h"
#include <Arduino.h>


void check_leds(){
  Serial.println("Checking LEDs");

  Serial.println("RED ON");
  for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){   
    ledcWrite(0, dutyCycle);
    delay(5);
  }

  Serial.println("RED OFF");
  for(int dutyCycle = 255; dutyCycle >=0; dutyCycle--){   
    ledcWrite(0, dutyCycle);
    delay(5);
  }


  Serial.println("GREEN ON");
  for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){   
    ledcWrite(1, dutyCycle);
    delay(5);
  }

  Serial.println("GREEN OFF");
  for(int dutyCycle = 255; dutyCycle >=0; dutyCycle--){   
    ledcWrite(1, dutyCycle);
    delay(5);
  }

  Serial.println("BLUE ON");
  for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){   
    ledcWrite(2, dutyCycle);
    delay(5);
  }

  Serial.println("BLUE OFF");
  for(int dutyCycle = 255; dutyCycle >=0; dutyCycle--){   
    ledcWrite(2, dutyCycle);
    delay(5);
  }
}

void check_vibro(){
  Serial.println("VIBRO ON");
  for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){   
    ledcWrite(3, dutyCycle);
    delay(2);
  }

  delay(500);

  Serial.println("VIBRO OFF");
  for(int dutyCycle = 255; dutyCycle >=0; dutyCycle--){   
    ledcWrite(3, dutyCycle);
    delay(2);
  }
}

void check_io(){
  Serial.println("Checking IO");
  digitalWrite(19, 0);//turn off output transistor
  
  Serial.print("Charger plugged out?");
  bool failure = false;
  for(int i=0;i<5 && !failure;i++){
    Serial.print(".");
    delay(100);
    if(!digitalRead(35)){
      failure = true;
    }
  }
  if(failure){
    Serial.println("FAIL");
  }else{
    Serial.println("OK");
  }

  Serial.print("Plug charger in.");
  bool success = false;
  for(int i=0;i<100 && !success;i++){
    Serial.print(".");
    delay(200);
    if(!digitalRead(35)){
      success = true;
    }
  }
  if(success){
    Serial.println("OK");
  }else{
    Serial.println("FAIL");
  }

  Serial.println("Plug charger out and connect voltmeter. It shoud read Zero");
  while(!digitalRead(35)){}
  delay(2000);
  
  if(!digitalRead(34)){
    Serial.println("Button should not read as pressed. FAIL");
  }

  Serial.print("Press button.");
  success = false;
  for(int i=0;i<100 && !success;i++){
    Serial.print(".");
    delay(200);
    if(!digitalRead(34)){
      success = true;
    }
  }
  if(success){
    Serial.println("OK");
  }else{
    Serial.println("FAIL");
  }

  digitalWrite(19, 1);//turn on output
  Serial.println("Read voltmeter readings. Have to match battery");
}

void check_leakage(){
  digitalWrite(18, 0);
  Serial.println("Leakage control off. Try touchig - should have no reaction.");
  delay(500);
  bool failure = false;
  for(int i=0;i<100 && !failure;i++){
    Serial.print(".");
    delay(200);
    if(!digitalRead(36)){
      failure = true;
    }
  }
  if(failure){
    Serial.println("FAIL");
  }else{
    Serial.println("OK");
  }

  digitalWrite(18, 1);
  Serial.println("Leakage control on. Try touchig.");
  delay(500);

  bool success = false;
  for(int i=0;i<50 && ! success;i++){
    Serial.print(".");
    delay(200);
    if(!digitalRead(36)){
      success = true;
    }
  }
  if(success){
    Serial.println("OK");
  }else{
    Serial.println("FAIL");
  }
}

//void printReg(uint8_t address){
//  // Serial.print("Register: 0x");
//  Serial.print(address, HEX);
//  Wire.beginTransmission(MAX1720X_ADDR);
//	Wire.write(address);
//	Wire.endTransmission(false);
//	Wire.requestFrom(MAX1720X_ADDR, (int)2);
//  uint16_t combined = Wire.read()|(Wire.read()<<8);     // LSB or-ed with MSB
//  Serial.print(", ");
//  // Serial.print(" value: 0x");
//  Serial.println(combined, HEX);
//  // Serial.print("  ");
//  // Serial.println(combined, BIN);
//}

