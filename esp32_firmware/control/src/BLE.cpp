#include "BLE.h"
#include "io.h"

#define SERVICE_UUID_ESPOTA                 "d804b643-6ce7-4e81-9f8a-ce0f699085eb"
#define CHARACTERISTIC_UUID_ID              "d804b644-6ce7-4e81-9f8a-ce0f699085eb"

#define SERVICE_UUID_OTA                    "c8659210-af91-4ad3-a995-a58d6fd26145" // UART service UUID
#define CHARACTERISTIC_UUID_FW              "c8659211-af91-4ad3-a995-a58d6fd26145"
#define CHARACTERISTIC_UUID_HW_VERSION      "c8659212-af91-4ad3-a995-a58d6fd26145"

#define SERVICE_VOLTAGES_UUID               "0b040846-b145-4eed-8205-651afbe2c281"
#define BLUETOOTH_VOLTAGE_1_UUID            "fbee111d-bdee-45c7-be4f-0844775b35f4"
#define BLUETOOTH_VOLTAGE_2_UUID            "e97583c3-eef5-4bf9-b81c-953be0524693"
#define BLUETOOTH_VOLTAGE_3_UUID            "65c9b4b3-0e0f-4cc2-aa89-70651a60f1f3"

#define BLUETOOTH_CURRENT_UUID              "83fb85bf-dd0d-4d2e-94f8-e24a8bde5c96"
#define BLUETOOTH_TEMPERATURE_UUID          "d97dac03-775e-499e-8b0e-0878885ea732"

#define SERVICE_UUID_HISTORY                "a25a9fc0-3d29-4940-9f7c-25be6a514f95"

#define HISTORY_REQUEST_UUID                "bac531e3-a370-4f5c-91b0-78ab1ce7602d" //send from device history packet ID to this UUID
#define CHARACTERISTIC_HISTORY_POSITION_UUID "67372646-059e-11eb-adc1-0242ac120002"

#define FULL_PACKET 512
#define CHARPOS_UPDATE_FLAG 5

esp_ota_handle_t otaHandler = 0;

bool updateFlag = false;
bool readyFlag = false;
int bytesReceived = 0;
volatile bool _BLEClientConnected = false;

void IRAM_ATTR otaCallback::onWrite(BLECharacteristic *pCharacteristic)
{
  std::string rxData = pCharacteristic->getValue();
  if (!updateFlag) { //If it's the first packet of OTA since bootup, begin OTA
    Serial.println("BeginOTA");
    esp_ota_begin(esp_ota_get_next_update_partition(NULL), OTA_SIZE_UNKNOWN, &otaHandler);
    updateFlag = true;
  }
  if (_p_ble != NULL)
  {
    if (rxData.length() > 0)
    {
      esp_ota_write(otaHandler, rxData.c_str(), rxData.length());
      if (rxData.length() != FULL_PACKET) //diry hack. it WILL fail if length is dividable by FULL_PACKET
      {
        esp_ota_end(otaHandler);
        Serial.println("EndOTA");
        if (ESP_OK == esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL))) {
          delay(2000);
          esp_restart();
        }
        else {
          Serial.println("Upload Error");
        }
      }
    }
  }

  uint8_t txData[5] = {1, 2, 3, 4, 5};
  //delay(1000);
  pCharacteristic->setValue((uint8_t*)txData, 5);
  pCharacteristic->notify();
}

void IRAM_ATTR historyCallback::onWrite(BLECharacteristic *pCharacteristic)
{
  std::string rxData = pCharacteristic->getValue();
  uint32_t data;
  if (rxData.length() == 4)
  {
    //Serial.println("Rx data: found");
    // Serial.println(rxData[0]);
    // Serial.println(rxData[1]);
    // Serial.println(rxData[2]);
    // Serial.println(rxData[3]);
    data = (rxData[3]<<24) + (rxData[2]<<16) + (rxData[1]<<8) + rxData[0];
    // Serial.println(data);
  //for some reason reading I2C in the callback always returns 0xFF, have to split into a main loop
    intention_to_read_address = data;
  }else{
    Serial.println("length does not match");
  }
}

//
// Constructor
BLE::BLE(void) {

}

//
// Destructor
BLE::~BLE(void)
{
  BLEDevice::deinit(true);
}

void BLE::end(){
  BLEDevice::deinit(true);
}

//
// begin
bool BLE::begin(const char* localName) {
  // Create the BLE Device
  BLEDevice::init(localName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new BLECustomServerCallbacks());

  // Create the BLE Service
  pESPOTAService = pServer->createService(SERVICE_UUID_ESPOTA);
  pService = pServer->createService(SERVICE_UUID_OTA);
  pHistoryService = pServer->createService(SERVICE_UUID_HISTORY);
  pSOC = pServer->createService(BLEUUID((uint16_t)BATTERY_SERVICE_UUID));

  pVoltages = pServer->createService(SERVICE_VOLTAGES_UUID);

  // Create a BLE Characteristic
  pESPOTAIdCharacteristic = pESPOTAService->createCharacteristic(
                                       CHARACTERISTIC_UUID_ID,
                                       BLECharacteristic::PROPERTY_READ
                                     );

  pVersionCharacteristic = pService->createCharacteristic(
                             CHARACTERISTIC_UUID_HW_VERSION,
                             BLECharacteristic::PROPERTY_READ
                           );

  pOtaCharacteristic = pService->createCharacteristic(
                         CHARACTERISTIC_UUID_FW,
                         BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
                       );

  pSOCCharacteristic = pSOC->createCharacteristic(
                         (uint16_t)0x2A19,
                         BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
                       );

  pHistoryCharacteristic = pHistoryService->createCharacteristic(HISTORY_REQUEST_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pHistoryPositionCharacteristic = pHistoryService->createCharacteristic(CHARACTERISTIC_HISTORY_POSITION_UUID, BLECharacteristic::PROPERTY_READ);

  pVoltage1Characteristic = pVoltages->createCharacteristic(BLUETOOTH_VOLTAGE_1_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  pVoltage2Characteristic = pVoltages->createCharacteristic(BLUETOOTH_VOLTAGE_2_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  pVoltage3Characteristic = pVoltages->createCharacteristic(BLUETOOTH_VOLTAGE_3_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);

  //serives are separated, cause all GATT params won't fit into a single one
  pCurrentCharacteristic = pSOC->createCharacteristic(BLUETOOTH_CURRENT_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  pTemperatureCharacteristic = pSOC->createCharacteristic(BLUETOOTH_TEMPERATURE_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);

  // BLEDescriptor BatteryLevelDescriptor(BLEUUID((uint16_t)0x2901));
  // BatteryLevelDescriptor.setValue("Percentage 0 - 100");    
  BLE2904* batteryLevelDescriptor = new BLE2904();
	batteryLevelDescriptor->setFormat(BLE2904::FORMAT_UINT8);
	batteryLevelDescriptor->setNamespace(1);
	batteryLevelDescriptor->setUnit(0x27ad);


  BLE2904* historyPositionDescriptor = new BLE2904();
	historyPositionDescriptor->setFormat(BLE2904::FORMAT_UINT32);
	historyPositionDescriptor->setNamespace(1);
	historyPositionDescriptor->setUnit(0x2760);//Time, minute
  

  BLE2904* batteryVoltage1Descriptor = new BLE2904();
	batteryVoltage1Descriptor->setFormat(BLE2904::FORMAT_FLOAT32);
	batteryVoltage1Descriptor->setNamespace(1);
	batteryVoltage1Descriptor->setUnit(0x2728);//Electrical Potential Difference, Voltage

  BLE2904* batteryVoltage2Descriptor = new BLE2904();
	batteryVoltage2Descriptor->setFormat(BLE2904::FORMAT_FLOAT32);
	batteryVoltage2Descriptor->setNamespace(1);
	batteryVoltage2Descriptor->setUnit(0x2728);//Electrical Potential Difference, Voltage

  BLE2904* batteryVoltage3Descriptor = new BLE2904();
	batteryVoltage3Descriptor->setFormat(BLE2904::FORMAT_FLOAT32);
	batteryVoltage3Descriptor->setNamespace(1);
	batteryVoltage3Descriptor->setUnit(0x2728);//Electrical Potential Difference, Voltage

  BLE2904* batteryCurrentDescriptor = new BLE2904();
	batteryCurrentDescriptor->setFormat(BLE2904::FORMAT_FLOAT32);
	batteryCurrentDescriptor->setNamespace(1);
	batteryCurrentDescriptor->setUnit(0x2704);//electric_current [ampere]


  BLE2904* batteryTemperatureDescriptor = new BLE2904();
	batteryTemperatureDescriptor->setFormat(BLE2904::FORMAT_FLOAT32);
	batteryTemperatureDescriptor->setNamespace(1);
	batteryTemperatureDescriptor->setUnit(0x2705);//thermodynamic_temperature [kelvin]

  pSOCCharacteristic->addDescriptor(batteryLevelDescriptor);
  pSOCCharacteristic->addDescriptor(new BLE2902());

  pVoltage1Characteristic->addDescriptor(batteryVoltage1Descriptor);
  pVoltage1Characteristic->addDescriptor(new BLE2902());

  pVoltage2Characteristic->addDescriptor(batteryVoltage2Descriptor);
  pVoltage2Characteristic->addDescriptor(new BLE2902());

  pVoltage3Characteristic->addDescriptor(batteryVoltage3Descriptor);
  pVoltage3Characteristic->addDescriptor(new BLE2902());

  pCurrentCharacteristic->addDescriptor(batteryCurrentDescriptor);
  pCurrentCharacteristic->addDescriptor(new BLE2902());

  pTemperatureCharacteristic->addDescriptor(batteryTemperatureDescriptor);
  pTemperatureCharacteristic->addDescriptor(new BLE2902());

  pOtaCharacteristic->addDescriptor(new BLE2902());
  pOtaCharacteristic->setCallbacks(new otaCallback(this));

  pHistoryCharacteristic->addDescriptor(new BLE2902());
  pHistoryCharacteristic->setCallbacks(new historyCallback(this));

  pHistoryPositionCharacteristic->addDescriptor(historyPositionDescriptor);
  pHistoryPositionCharacteristic->addDescriptor(new BLE2902());

  // Start the service(s)
  pESPOTAService->start();
  pService->start();
  pHistoryService->start();
  pSOC->start();
  pVoltages->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID_ESPOTA);
  pAdvertising->addServiceUUID(SERVICE_UUID_HISTORY);
  pAdvertising->addServiceUUID(BLEUUID((uint16_t)BATTERY_SERVICE_UUID));
  BLEDevice::startAdvertising();

  uint8_t hardwareVersion[5] = {HARDWARE_VERSION_MAJOR, HARDWARE_VERSION_MINOR, SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_PATCH};
  pVersionCharacteristic->setValue((uint8_t*)hardwareVersion, 5);

  return true;
}

void BLE::set_log_position(uint32_t log_position){
  pHistoryPositionCharacteristic->setValue(log_position);
}

void BLE::notify_ble(log_record &record){

  Serial.print("Notifying BLE SOC:");
  Serial.println(record.soc);

  pSOCCharacteristic->setValue(&record.soc, 1);
  pSOCCharacteristic->notify();

  pVoltage1Characteristic->setValue(record.v1);
  pVoltage1Characteristic->notify();

  pVoltage2Characteristic->setValue(record.v2);
  pVoltage2Characteristic->notify();

  pVoltage3Characteristic->setValue(record.v3);
  pVoltage3Characteristic->notify();

  pCurrentCharacteristic->setValue(record.current);
  pCurrentCharacteristic->notify();

  pTemperatureCharacteristic->setValue(record.temperature);
  pTemperatureCharacteristic->notify();
}

void BLE::notify_history(log_record &record){
  pHistoryCharacteristic->setValue((uint8_t*)(void*)&record, sizeof(log_record));
  pHistoryCharacteristic->notify();
}