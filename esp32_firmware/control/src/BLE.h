#ifndef _BLE_H_
#define _BLE_H_

#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2904.h>

#include "esp_ota_ops.h"
#include "max1720x.h"
#include "globals.h"

#define SOFTWARE_VERSION_MAJOR 0
#define SOFTWARE_VERSION_MINOR 1
#define SOFTWARE_VERSION_PATCH 2
#define HARDWARE_VERSION_MAJOR 6
#define HARDWARE_VERSION_MINOR 5

#define BATTERY_SERVICE_UUID 0x180F

extern volatile bool _BLEClientConnected;

class BLE; // forward declaration

class BLECustomServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      _BLEClientConnected = true;
      Serial.println("BLE connected");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("BLE disconnected");
      _BLEClientConnected = false;
    }
};


class otaCallback: public BLECharacteristicCallbacks {
  public:
    otaCallback(BLE* ble) {
      _p_ble = ble;
    }
    BLE* _p_ble;

    void IRAM_ATTR onWrite(BLECharacteristic *pCharacteristic);
};

class historyCallback: public BLECharacteristicCallbacks {
  public:
    historyCallback(BLE* ble) {
      _p_ble = ble;
    }
    BLE* _p_ble;

    void IRAM_ATTR onWrite(BLECharacteristic *pCharacteristic);
};

class BLE
{
  public:

    BLE(void);
    ~BLE(void);

    bool begin(const char* localName);
    void end();

    void notify_ble(log_record &record);
    void notify_history(log_record &record);
    void set_log_position(uint32_t log_position);
  
  private:
    String local_name;

    BLEServer *pServer = NULL;

    BLEService *pESPOTAService = NULL;
    BLEService *pHistoryService = NULL;
    BLEService *pSOC = NULL;
    BLEService *pVoltages = NULL;
    BLEService *pService = NULL;

    BLECharacteristic * pESPOTAIdCharacteristic = NULL;
    BLECharacteristic * pVersionCharacteristic = NULL;
    BLECharacteristic * pOtaCharacteristic = NULL;
    BLECharacteristic * pSOCCharacteristic = NULL;
    BLECharacteristic * pVoltage1Characteristic = NULL;
    BLECharacteristic * pVoltage2Characteristic = NULL;
    BLECharacteristic * pVoltage3Characteristic = NULL;
    BLECharacteristic * pCurrentCharacteristic = NULL;
    BLECharacteristic * pTemperatureCharacteristic = NULL;
    BLECharacteristic * pHistoryCharacteristic = NULL;
    BLECharacteristic * pHistoryPositionCharacteristic = NULL;
};

#endif
