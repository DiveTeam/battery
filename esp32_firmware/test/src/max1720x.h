/**
 * Name: max1720x
 * Author: Luka Mustafa - Institute IRNAS Race { info@irnas.eu }
 * Version: 1.0
 * Description: A library for interfacing the MAXIM MAX17201/MAX17205
 * 				Li+ fuel gauges.
 * Source: https://github.com/pAIgn10/LiFuelGauge
 * License: Copyright (c) 2017 Nick Lamprianidis 
 *          This library is licensed under the GPL license
 *          http://www.opensource.org/licenses/mit-license.php
 * Inspiration: The library is inspired by: https://github.com/pAIgn10/LiFuelGauge
 * Filename: max1720x.h
 * File description: Definitions and methods for the max1720x library
 */

#ifndef max1720x_h
#define max1720x_h

#include <Arduino.h>
#include <Wire.h>

// MAX1720X register addresses
const int MAX1720X_ADDR = 0x36;
const int MAX1720X_SHADOW_RAM_ADDR = 0x0B;
const int MAX1720X_STATUS_ADDR = 0x00;          // Contains alert status and chip status
const int MAX1720X_AVGCURENT_ADDR = 0x0B;       // Average current register
const int MAX1720X_CELL1_ADDR = 0xD8;           // Cell1 voltage
const int MAX1720X_CELL2_ADDR = 0xD7;           // Cell1 voltage
const int MAX1720X_CELL3_ADDR = 0xD6;           // Cell1 voltage
const int MAX1720X_MAXMIN_V_ADDR = 0x01B;       // Max min register for voltage
const int MAX1720X_REPSOC_ADDR = 0x06;          // Reported state of charge
const int MAX1720X_REPCAP_ADDR = 0x05;          // Reported remaining capacity
const int MAX1720X_TEMP_ADDR = 0x08;            // Temperature
const int MAX1720X_CURENT_ADDR = 0x0A;          // Battery current
const int MAX1720X_MAXMIN_CURENT_ADDR = 0x1C;   // max min register for current
const int MAX1720X_TTE_ADDR = 0x11;             // Time to empty
const int MAX1720X_TTF_ADDR = 0x20;             // Time to full
const int MAX1720X_CAPACITY_ADDR = 0x10;        // Full capacity estimation
const int MAX1720X_VBAT_ADDR = 0xDA;            // Battery pack voltage
const int MAX1720X_AVCELL_ADDR = 0x17;          // Battery cycles
const int MAX1720X_COMMAND_ADDR = 0x60;         // Command register
const int MAX1720X_CONFIG2_ADDR = 0xBB;         // Command register
const double RSENSE = 0.002;                    // SHUNT value, Ohms

// Class for interfacing the MAX1720X Li+ fuel gauges
class max1720x
{
public:
	max1720x();
    double    getCellVoltage(uint8_t cell);
    double    getPackVoltage();
    double    getSOC();
    double    getTemperature();
    double    getCurrent();
    double    getAvgCurrent();
    double    getMaxCurrent();
    double    getMinCurrent();
    double    getCapacity();
    double    getTTE();
    double    getTTF();
    uint8_t   fullReset();
    uint8_t   fuelReset();
    uint8_t   getStatus();

    uint8_t   resetMaxMinAvgCurrent();
    uint8_t   writeReg(uint8_t address, uint16_t value);
    uint8_t   writeShadowReg(uint8_t address, uint16_t value);
    // void      setCurrentAvgTime(int input_value);
};

#endif  // max1720x
