#include "io.h"
#include "soc/ledc_reg.h"
#include "soc/ledc_struct.h"
#include "max1720x.h"
// #include <Arduino.h>
// #include <Wire.h>
#include <extEEPROM.h>

#define LEDC_CHAN(g,c) LEDC.channel_group[(g)].channel[(c)]
#define LEDC_TIMER(g,t) LEDC.timer_group[(g)].timer[(t)]

#define VIBRO_PIN 5

#define EEPROM_ADDR_POSITION_COUNTER 0
#define EEPROM_BMS_MAGIC_MARK_ADDRESS 10

extEEPROM myEEPROM(kbits_2048, 1, 256); //single EEPROM, 256 bytes page
max1720x gauge;

void setup_led(){
  ledcSetup(0, 5000, 8); //setting up PWM (3 channels) at 5Hz frequency with 8-bit resolution
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcAttachPin(26, 0);//RED
  ledcAttachPin(25, 1);//GREEN
  ledcAttachPin(27, 2);//BLUE
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
  ledcWrite(3, 0);
}

void save_bms_config(){//WARNING: BMS memery can be written only 7 (SEVEN) times. Be wise, don't waste
  Wire.beginTransmission(MAX1720X_ADDR);
  Wire.write(0x60);                                   // write E904h to 60h as described in datasheet 
  Wire.write(0x04);
  Wire.write(0xe9);
  Wire.endTransmission(true);
  delay(1000);
  gauge.fuelReset();
  delay(500);
}

void bms_initial_setup(){ //only call once
  Serial.println("Configuring BMS.");
  gauge.fuelReset();
  sleep(100);
  gauge.fullReset();                                      // Resets MAX1720x
  delay(1000);

  gauge.writeReg(0x00, 0x0000);		//clear status register
  gauge.writeReg(0xBD, 0x3A63);		//nPackCfg Register - temporary (0x9663 is not working at this stage)

  gauge.writeReg(0x18,0x2530); //set design capacity to 23800
  gauge.writeReg(0x23,0x2530); //set design capacity to 23800

  gauge.writeShadowReg(0x80, 0x00F0);		//nXTable0 Register
  gauge.writeShadowReg(0x81, 0x0686);		//nXTable1 Register
  gauge.writeShadowReg(0x82, 0x071E);		//nXTable2 Register
  gauge.writeShadowReg(0x83, 0x06E2);		//nXTable3 Register
  gauge.writeShadowReg(0x84, 0x3AED);		//nXTable4 Register
  gauge.writeShadowReg(0x85, 0x3104);		//nXTable5 Register
  gauge.writeShadowReg(0x86, 0x1650);		//nXTable6 Register
  gauge.writeShadowReg(0x87, 0x1659);		//nXTable7 Register
  gauge.writeShadowReg(0x88, 0x0923);		//nXTable8 Register
  gauge.writeShadowReg(0x89, 0x0930);		//nXTable9 Register
  gauge.writeShadowReg(0x8A, 0x0816);		//nXTable10 Register
  gauge.writeShadowReg(0x8B, 0x06EE);		//nXTable11 Register
  gauge.writeShadowReg(0x8C, 0x0000);		//nUser18C Register
  gauge.writeShadowReg(0x8D, 0x0000);		//nUser18D Register
  gauge.writeShadowReg(0x8E, 0x0000);		//nODSCTh Register
  gauge.writeShadowReg(0x8F, 0x0000);		//nODSCCfg Register
  gauge.writeShadowReg(0x90, 0x961B);		//nOCVTable0 Register
  gauge.writeShadowReg(0x91, 0xAD16);		//nOCVTable1 Register
  gauge.writeShadowReg(0x92, 0xB2AB);		//nOCVTable2 Register
  gauge.writeShadowReg(0x93, 0xB8CB);		//nOCVTable3 Register
  gauge.writeShadowReg(0x94, 0xBADB);		//nOCVTable4 Register
  gauge.writeShadowReg(0x95, 0xBB28);		//nOCVTable5 Register
  gauge.writeShadowReg(0x96, 0xBC5C);		//nOCVTable6 Register
  gauge.writeShadowReg(0x97, 0xBDE2);		//nOCVTable7 Register
  gauge.writeShadowReg(0x98, 0xBF78);		//nOCVTable8 Register
  gauge.writeShadowReg(0x99, 0xC59D);		//nOCVTable9 Register
  gauge.writeShadowReg(0x9A, 0xCC08);		//nOCVTable10 Register
  gauge.writeShadowReg(0x9B, 0xD01A);		//nOCVTable11 Register
  gauge.writeShadowReg(0x9C, 0x0BE6);		//nIChgTerm Register
  gauge.writeShadowReg(0x9D, 0x0000);		//nFilterCfg Register
  gauge.writeShadowReg(0x9E, 0x965A);		//nVEmpty Register
  gauge.writeShadowReg(0x9F, 0xA602);		//nLearnCfg Register
  gauge.writeShadowReg(0xA0, 0x1A2F);		//nQRTable00 Register
  gauge.writeShadowReg(0xA1, 0x1913);		//nQRTable10 Register
  gauge.writeShadowReg(0xA2, 0x1886);		//nQRTable20 Register
  gauge.writeShadowReg(0xA3, 0x1401);		//nQRTable30 Register
  gauge.writeShadowReg(0xA4, 0x0000);		//nCycles Register
  gauge.writeShadowReg(0xA5, 0x2530);		//nFullCapNom Register
  gauge.writeShadowReg(0xA6, 0x1070);		//nRComp0 Register
  gauge.writeShadowReg(0xA7, 0x223E);		//nTempCo Register
  gauge.writeShadowReg(0xA8, 0x4610);		//nIAvgEmpty Register
  gauge.writeShadowReg(0xA9, 0x2530);		//nFullCapRep Register
  gauge.writeShadowReg(0xAA, 0x0000);		//nVoltTemp Register
  gauge.writeShadowReg(0xAB, 0x807F);		//nMaxMinCurr Register
  gauge.writeShadowReg(0xAC, 0x00FF);		//nMaxMinVolt Register
  gauge.writeShadowReg(0xAD, 0x807F);		//nMaxMinTemp Register
  gauge.writeShadowReg(0xAE, 0x0000);		//nSOC Register
  gauge.writeShadowReg(0xAF, 0x0000);		//nTimerH Register
  gauge.writeShadowReg(0xB0, 0x0211);		//nConfig Register
  gauge.writeShadowReg(0xB1, 0x0204);		//nRippleCfg Register
  gauge.writeShadowReg(0xB2, 0x3070);		//nMiscCfg Register
  gauge.writeShadowReg(0xB3, 0x2530);		//nDesignCap Register
  gauge.writeShadowReg(0xB4, 0x0000);		//nHibCfg Register
  gauge.writeShadowReg(0xB5, 0x9C63);		//nPackCfg Register 
  gauge.writeShadowReg(0xB6, 0x0000);		//nRelaxCfg Register
  gauge.writeShadowReg(0xB7, 0x2241);		//nConvgCfg Register
  gauge.writeShadowReg(0xB8, 0xC9B0);		//nNVCfg0 Register
  gauge.writeShadowReg(0xB9, 0xA006);		//nNVCfg1 Register
  gauge.writeShadowReg(0xBA, 0xFE0A);		//nNVCfg2 Register
  gauge.writeShadowReg(0xBB, 0x0002);		//nSBSCfg Register
  gauge.writeShadowReg(0xBC, 0x0000);		//nROMID0 Register
  gauge.writeShadowReg(0xBD, 0x0000);		//nROMID1 Register
  gauge.writeShadowReg(0xBE, 0x0000);		//nROMID2 Register
  gauge.writeShadowReg(0xBF, 0x0000);		//nROMID3 Register
  gauge.writeShadowReg(0xC0, 0x0000);		//nVAlrtTh Register
  gauge.writeShadowReg(0xC1, 0x0000);		//nTAlrtTh Register
  gauge.writeShadowReg(0xC2, 0x0000);		//nSAlrtTh Register
  gauge.writeShadowReg(0xC3, 0x0000);		//nIAlrtTh Register
  gauge.writeShadowReg(0xC4, 0x0000);		//nUser1C4 Register
  gauge.writeShadowReg(0xC5, 0x0000);		//nUser1C5 Register
  gauge.writeShadowReg(0xC6, 0x5505);		//nFullSOCThr Register
  gauge.writeShadowReg(0xC7, 0x0000);		//nTTFCfg Register
  gauge.writeShadowReg(0xC8, 0x0000);		//nCGain Register
  gauge.writeShadowReg(0xC9, 0x0064);		//nTCurve Register
  gauge.writeShadowReg(0xCA, 0xF49A);		//nTGain Register
  gauge.writeShadowReg(0xCB, 0x16A1);		//nTOff Register
  gauge.writeShadowReg(0xCC, 0x0000);		//nManfctrName0 Register
  gauge.writeShadowReg(0xCD, 0x0000);		//nManfctrName1 Register
  gauge.writeShadowReg(0xCE, 0x0000);		//nManfctrName2 Register
  gauge.writeShadowReg(0xCF, 0x03E8);		//nRSense Register
  gauge.writeShadowReg(0xD0, 0x0000);		//nUser1D0 Register
  gauge.writeShadowReg(0xD1, 0x0000);		//nUser1D1 Register
  gauge.writeShadowReg(0xD2, 0xD5E3);		//nAgeFcCfg Register
  gauge.writeShadowReg(0xD3, 0x0000);		//nDesignVoltage Register
  gauge.writeShadowReg(0xD4, 0x0000);		//nUser1D4 Register
  gauge.writeShadowReg(0xD5, 0x0000);		//nRFastVShdn Register
  gauge.writeShadowReg(0xD6, 0x0000);		//nManfctrDate Register
  gauge.writeShadowReg(0xD7, 0x0000);		//nFirstUsed Register
  gauge.writeShadowReg(0xD8, 0x0000);		//nSerialNumber0 Register
  gauge.writeShadowReg(0xD9, 0x0000);		//nSerialNumber1 Register
  gauge.writeShadowReg(0xDA, 0x0000);		//nSerialNumber2 Register
  gauge.writeShadowReg(0xDB, 0x0000);		//nDeviceName0 Register
  gauge.writeShadowReg(0xDC, 0x0000);		//nDeviceName1 Register
  gauge.writeShadowReg(0xDD, 0x0000);		//nDeviceName2 Register
  gauge.writeShadowReg(0xDE, 0x0000);		//nDeviceName3 Register
  gauge.writeShadowReg(0xDF, 0x0000);		//nDeviceName4 Register

  save_bms_config();

  gauge.writeReg( 0xBB, 0x0001 ); // fuel gauge reset
  delay(500);
  gauge.fuelReset();
  delay(500);
  Serial.println("BMS config saved. Remove all power sources and connect everything back.");
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

bool is_bms_config_magic_mark_stored(){
  byte writeData[10] = { 9, 8, 7, 6, 5, 5, 6, 7, 8, 9};
  byte readData[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  myEEPROM.read(EEPROM_BMS_MAGIC_MARK_ADDRESS, readData, 10);
  
  for(int i=0; i < 10; i++){
    if(readData[i] != writeData[i]){
      return false;
    }
  }
  return true;
}

void make_bms_config_magic_mark(){
  Serial.println("Storing magic BMS config mark in EEPROM to avoid accidental rewtire");
  byte writeData[10] = { 9, 8, 7, 6, 5, 5, 6, 7, 8, 9};
  //write 10 bytes starting at location 42
  myEEPROM.write(EEPROM_BMS_MAGIC_MARK_ADDRESS, writeData, 10);
}

void reset_position_counter(){
  byte zero_array[4]={0,0,0,0}; //reset log counter to 0
  myEEPROM.write(EEPROM_ADDR_POSITION_COUNTER, zero_array, sizeof(zero_array));
}

bool check_eeprom(){
  Serial.println("Checking EEPROM");

  Serial.println("Writing 10 bytes starting from address 42");
  byte writeData[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  //write 10 bytes starting at location 42
  byte i2cStat = myEEPROM.write(42, writeData, 10);
  if ( i2cStat != 0 ) {
    if ( i2cStat == EEPROM_ADDR_ERR) {
      Serial.println("Error writing data - bad address");
      return false;
    }
    else {
      Serial.println("Error writing data - I2C error");
      return false;
    }
  }

  Serial.println("Reading 10 bytes");
  byte readData[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  //read 10 bytes starting at location 42
  i2cStat = myEEPROM.read(42, readData, 10);
  if ( i2cStat != 0 ) {
    if ( i2cStat == EEPROM_ADDR_ERR) {
      Serial.println("Error reading data - bad address");
      return false;
    }
    else {
      Serial.println("Error reading data - I2C error");
      return false;
    }
  }

  Serial.print("Verification...");

  bool error = false;
  for(int i=0; i < 10; i++){
    if(readData[i] != writeData[i]){
      error = true;
    }
  }
  if(error){
    Serial.println("FAIL");
    return false;
  }else{
    Serial.println("OK");
  }
  for(int i=0; i < 10; i++){
    Serial.print(readData[i], HEX);
    Serial.print(", ");
  }
  Serial.println();
  if(error){
    return false;
  }else{
    return true;
  }
}


void check_bms(){
  digitalWrite(19, 1);//turn on output
  delay(100);
  Serial.println("Checking BMS");
  if(digitalRead(39)){
    Serial.println("Alert HIGH - no alert");
  }else{
    Serial.println("Alert LOW");
  }
  Serial.print("Capacity: ");
  Serial.print(gauge.getCapacity());                  // Gets the battery state of charge
  Serial.println(" mAh");
  Serial.print("TTE: ");
  Serial.print(gauge.getTTE());                       // Gets the battery state of charge
  Serial.println(" s"); 
  Serial.print("TTF: ");
  Serial.print(gauge.getTTF());                       // Gets the battery state of charge
  Serial.println(" s");
  Serial.print("Current: ");
  Serial.print(gauge.getCurrent());                   // Gets the battery state of charge
  Serial.println(" mA");
  Serial.print("Average current: ");
  Serial.print(gauge.getAvgCurrent());                // Gets the battery state of charge
  Serial.println(" mA");
  Serial.print("Max current: ");
  Serial.print(gauge.getMaxCurrent());                // Gets the battery max current since reset
  Serial.println(" mA");
  Serial.print("Min Current: ");
  Serial.print(gauge.getMinCurrent());                // Gets the battery min current since reset
  Serial.println(" mA");
  Serial.print("Temperature: ");
  Serial.print(gauge.getTemperature());               // Gets the battery's state of charge
  Serial.println(" degC");
  Serial.print("SOC: ");
  Serial.print(gauge.getSOC());                       // Gets the battery's state of charge
  Serial.println("%");
  Serial.print("V1: ");
  Serial.print(gauge.getCellVoltage(0));              // Gets the Cell1 voltage
  Serial.println("mV");
  Serial.print("V2: ");
  Serial.print(gauge.getCellVoltage(1));              // Gets the Cell2 voltage
  Serial.println("mV");
  Serial.print("V3: ");
  Serial.print(gauge.getCellVoltage(2));              // Gets the Cell3 voltage
  Serial.println("mV");
  Serial.print("Pack Voltage: ");
  Serial.print(gauge.getPackVoltage());               // Gets the battery voltage
  Serial.println("mV");
  
  Serial.print("Battery status: ");
  Serial.print(gauge.getStatus(), BIN);
  Serial.println("");


  // for(int address=0x00;address<0x00+16;address++){printReg(address);}
  // for(int address=0x10;address<0x10+16;address++){printReg(address);}
  // for(int address=0x20;address<0x20+16;address++){printReg(address);}
  // for(int address=0x30;address<0x30+16;address++){printReg(address);}
  // for(int address=0x40;address<0x40+16;address++){printReg(address);}
  // for(int address=0xB0;address<0xB0+16;address++){printReg(address);}
  // for(int address=0xD0;address<0xD0+16;address++){printReg(address);}
}
