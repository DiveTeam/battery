/**
 * Name: max1720x
 * Author: Luka Mustafa - Institute IRNAS Race { info@irnas.eu }
 * Version: 1.0
 * Description: A library for interfacing the MAXIM MAX17201/MAX17205
 * 				Li+ fuel gauges.
 * Source: https://github.com/pAIgn10/max1720x
 * License: Copyright (c) 2017 Nick Lamprianidis 
 *          This library is licensed under the GPL license
 *          http://www.opensource.org/licenses/mit-license.php
 * Inspiration: The library is inspired by: https://github.com/pAIgn10/max1720x
 * Filename: max1720x.cpp
 * File description: Definitions and methods for the max1720x library
 */

#include "max1720x.h"

// Initializes variables and the Wire library
max1720x::max1720x() { 
    Wire.begin(); 
}

/*
 *  Function:     double max1720x::getCellVoltage()
 *  Description:  gets cell voltage and returns it in mV. Accepts 0,1 or 2
 */
double max1720x::getCellVoltage(uint8_t cell)
{
  if(cell>2){
    return 0;
  }
	Wire.beginTransmission(MAX1720X_ADDR);

  int address = MAX1720X_CELL1_ADDR;
  if(cell == 1){ address = MAX1720X_CELL2_ADDR; }
  if(cell == 2){ address = MAX1720X_CELL3_ADDR; }
	Wire.write(address);
	Wire.endTransmission(false);
	Wire.requestFrom(MAX1720X_ADDR, (int)2,HIGH);         //send stop
  uint16_t combined = Wire.read()|(Wire.read()<<8);     // LSB or-ed with MSB
  double voltage = combined;                            //combine registers
  return voltage*0.078125;                              //calculate actual value and return in mV
}


/*
 *  Function:     double max1720x::getPackVoltage()
 *  Description:  gets pack voltage and returns it in mV
 */
double max1720x::getPackVoltage()
{
	Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_VBAT_ADDR);
	Wire.endTransmission(false);
	Wire.requestFrom(MAX1720X_ADDR, (int)2,HIGH);         //send stop
  uint16_t combined = Wire.read()|(Wire.read()<<8);     // LSB or-ed with MSB
  double voltage = combined;                            //combine registers
  return voltage*1.25;                              //calculate actual value and return in mV
}


/*
 *  Function:     double max1720x::getCurrent()
 *  Description:  gets the voltage between two pins and calculates it with 0.0015625 mV/Ohm to get mA
 */
double max1720x::getCurrent()
{
	Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_CURENT_ADDR);
	Wire.endTransmission(false);
	Wire.requestFrom(MAX1720X_ADDR, (int)2,HIGH);         //send stop
  int16_t combined = Wire.read()|(Wire.read()<<8);      // LSB or-ed with MSB
  double current = (double)combined*0.0015625/RSENSE;    //calculate actual value as 0.0015625 mV/Ohm
  return current;                                   
}

/*
 *  Function:     double max1720x::getAvgCurrent()
 *  Description:  gets the average from the average register and calculates it with 0.0015625 mV/Ohm to get mA
 */
double max1720x::getAvgCurrent()
{
 Wire.beginTransmission(MAX1720X_ADDR);
 Wire.write(MAX1720X_AVGCURENT_ADDR);
 Wire.endTransmission(false);
 Wire.requestFrom(MAX1720X_ADDR, (int)2, HIGH);
 int16_t combined  = Wire.read()|(Wire.read()<<8);      // LSB or-ed with MSB
 double current = (double)combined*0.0015625/RSENSE;
 return current;                                        //calculate actual value as 0.0015625 mV/Ohm
}

/*
 *  Function:     double max1720x::getMaxCurrent()
 *  Description:  gets the maxmin current register and returns max
 */
double max1720x::getMaxCurrent()
{
  /*
    0.4mV / Rsense resolution
   */
   
  Wire.beginTransmission(MAX1720X_ADDR);
  Wire.write(MAX1720X_MAXMIN_CURENT_ADDR);
  Wire.endTransmission(false);
  Wire.requestFrom(MAX1720X_ADDR, (int)2,HIGH);         //send stop
  int8_t minimum     = Wire.read();                     // getting max
  int8_t maximum     = Wire.read();                     // getting min

  return ((double)maximum * 0.4 / RSENSE);               // calculate it with 
}

/*
 *  Function:     double max1720x::getMinCurrent()
 *  Description:  gets the maxmin current register and returns min
 */
double max1720x::getMinCurrent()
{
  /*
    .0004mV / Rsense resolution
    0.0004V / 0.010ohm resolution
    0.04 resolution
    that is 40mA resolution!
   */
   
  Wire.beginTransmission(MAX1720X_ADDR);
  Wire.write(MAX1720X_MAXMIN_CURENT_ADDR);
  Wire.endTransmission(false);
  Wire.requestFrom(MAX1720X_ADDR, (int)2,HIGH);         //send stop
  int8_t minimum     = Wire.read();                     // getting max
  int8_t maximum     = Wire.read();                     // getting min

  return ((double)minimum * 0.04 * 1000);               // calculate it with
}

/*
 *  Function:     double max1720x::getTemperature()
 *  Descritpion:  gets the temperature from the temperature register
 */
double max1720x::getTemperature()
{
	Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_TEMP_ADDR);
	Wire.endTransmission(false);
	Wire.requestFrom(MAX1720X_ADDR, (int)2,HIGH);         //send stop
  int16_t combined = Wire.read()|(Wire.read()<<8);      // LSB or-ed with MSB
  double temperature = (double)combined/256;            // to get the right value
  return temperature;
}

/*
 *  Function:     double max1720x::getSOC()
 *  Description:  Returns the relative state of charge of the connected LiIon Polymer battery as a percentage of the full capacity w/ resolution 1/256%
 */
double max1720x::getSOC()
{
	Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_REPSOC_ADDR);
	Wire.endTransmission(false);
	Wire.requestFrom(MAX1720X_ADDR, (int)2);
  uint16_t combined = Wire.read()|(Wire.read()<<8);     // LSB or-ed with MSB
  double soc = (double)combined/256;                                //combine registers
	return soc;                                       //calculate actual value and return in %
}

/*
 *  Function: double max1720x::getCapacity()
 *  Description: RepCap or reported capacity is a filtered version of the AvCap register that prevents large jumps in the reported value caused by changes in the application such as abrupt changes in temperature or load current. 
 */
double max1720x::getCapacity()
{
	Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_REPCAP_ADDR);
	Wire.endTransmission(false);
	Wire.requestFrom(MAX1720X_ADDR, (int)2);
  uint16_t combined = Wire.read()|(Wire.read()<<8);     // LSB or-ed with MSB
	double capacity = (double)combined*0.005/RSENSE;        //calculate actual value as 0.005 mVh/Ohm
  return capacity;
}

/*
 *  Function:     double max1720x::getTTE()
 *  Description:  The TTE register holds the estimated time to empty for the application under present temperature and load conditions 
 */
double max1720x::getTTE()
{
	Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_TTE_ADDR);
	Wire.endTransmission(false);
	Wire.requestFrom(MAX1720X_ADDR, (int)2);
  uint16_t combined = Wire.read()|(Wire.read()<<8);     // LSB or-ed with MSB
	double capacity = (double)combined*5.625;             // calculate actual value as value*5.625s
  return capacity;
}

/*
 *  Function:     double max1720x::getTTF()
 *  Description:  The TTF register holds the estimated time to full for the application under present conditions. 
 */
double max1720x::getTTF()
{
	Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_TTF_ADDR);
	Wire.endTransmission(false);
	Wire.requestFrom(MAX1720X_ADDR, (int)2);
  uint16_t combined = Wire.read()|(Wire.read()<<8);     // LSB or-ed with MSB
	double capacity = (double)combined*5.625;             //calculate actual value as value*5.625s
  return capacity;
}

/*
 *  Function:     uint8_t max1720x::getStatus()
 *  Description:  reads the status register
 */
uint8_t max1720x::getStatus()
{
	Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_STATUS_ADDR);
	Wire.endTransmission(false);
	Wire.requestFrom(MAX1720X_ADDR, (int)2);
  uint16_t combined = Wire.read()|(Wire.read()<<8);     // LSB or-ed with MSB
	return combined;
}

/*
 *  Function:     uint8_t max1720x::resetMaxMinAvgCurrent()
 *  Description:  Resets the maxmin avg current register by writing 0x807F into it
 */
uint8_t max1720x::resetMaxMinAvgCurrent()
{
  Wire.beginTransmission(MAX1720X_ADDR);
  Wire.write(MAX1720X_MAXMIN_CURENT_ADDR);              // go to register
  Wire.write((0x807F >> 0) & 0xFF);                     // write first half
  Wire.write((0x807F >> 8) & 0xFF);                     // write second half
  return Wire.endTransmission();
}

/*
 *  Function:     void max1720x::setCurrentAvgTime(int input_value)
 *  Description:  sets the time for avg current defined by the table
 */
/*void max1720x::setCurrentAvgTime(int input_value)
{
  // the formula is calculated by 45 * 2^(value-7) = x seconds

  // table of seconds
  // 0 -> 0,35s    3 -> 2,8s    6 -> 22,5s  9  -> 3min   12 -> 24min   15 -> 3,2h
  // 1 -> 0,703s   4 -> 5,65s   7 -> 45s    10 -> 6min   13 -> 48min
  // 2 -> 1,40     5 -> 11,12s  8 -> 90s    11 -> 12min  14 -> 1,6h

  Wire.beginTransmission(MAX1720X_ADDR);
  Wire.write(0x29);
  Wire.endTransmission(false);
  Wire.requestFrom(MAX1720X_ADDR, (int)2);
  uint16_t combined = Wire.read()|(Wire.read()<<8);      // LSB or-ed with MSB
  combined = combined & 0xFFF0;                          // with this we are clearing the 4 bits we want to set after
  
  
  // with (value & 0x000F) we make it copy the good values but set everything else to 0 
  // with current_reg | (value & 0x000F) we make it "copy" the other values from the current reg to the value
  
  uint16_t new_reg = combined | (input_value & 0x000F);  
  delay(100);
  Wire.beginTransmission(MAX1720X_ADDR);
  Wire.write(0x29);                                      // go to register
  Wire.write((new_reg >> 0) & 0xFF);                     // first half
  Wire.write((new_reg >> 8) & 0xFF);                     // second half
  Wire.endTransmission();  
}
*/

/*
 *  Function: uint8_t max1720x::fullReset()
 *  Description: resets the chip
 */
uint8_t max1720x::fullReset()
{
 	Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_COMMAND_ADDR);
	Wire.write(0x0f);
	Wire.write(0x00);
  Wire.endTransmission();
  delay(50);
	fuelReset();
  delay(50);
}

/*
  reset onlyu fuel gauge
*/
uint8_t max1720x::fuelReset()
{
  Wire.beginTransmission(MAX1720X_ADDR);
	Wire.write(MAX1720X_CONFIG2_ADDR);
	Wire.write(0x01);
	Wire.write(0x00);
	return Wire.endTransmission();
}

uint8_t max1720x::writeReg(uint8_t address, uint16_t value){
  Serial.print("Setting reg ");
  Serial.print(address, HEX);
  Serial.print("to ");
  Serial.print(value, HEX);
  Serial.print(" BIN: ");
  Serial.println(value, BIN);

  Wire.beginTransmission(MAX1720X_ADDR);
  Wire.write(address);                                 // go to register
  Wire.write((value >> 0) & 0xFF);                     // write first half
  Wire.write((value >> 8) & 0xFF);                     // write second half

  uint8_t response = Wire.endTransmission();
  Serial.print("Response: ");
  Serial.println(response, HEX);
  return response;
}

uint8_t max1720x::writeShadowReg(uint8_t address, uint16_t value){
  Serial.print("Setting shadow ");
  Serial.print(address, HEX);
  Serial.print(" to ");
  Serial.print(value, HEX);
  Serial.print(" BIN: ");
  Serial.println(value, BIN);
  
  Wire.beginTransmission(MAX1720X_SHADOW_RAM_ADDR);
  Wire.write(address);                                 // go to register
  Wire.write((value >> 0) & 0xFF);                     // write first half
  Wire.write((value >> 8) & 0xFF);                     // write second half
  
  uint8_t response = Wire.endTransmission();
  Serial.print("Response: ");
  Serial.println(response, HEX);
  return response;
}
