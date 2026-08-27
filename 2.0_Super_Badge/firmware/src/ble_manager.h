#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

extern void processCommand(String data);

static NimBLEServer *pServer = NULL;
static NimBLECharacteristic * pTxCharacteristic = NULL;
static bool deviceConnected = false;

class MyServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE Connected!");
    };
    void onDisconnect(NimBLEServer* pServer) {
      deviceConnected = false;
      Serial.println("BLE Disconnected!");
      // Restart advertising
      NimBLEDevice::startAdvertising();
    }
};

class MyCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        String data = String(rxValue.c_str());
        data.trim();
        if(data.length() > 0) {
            Serial.println("BLE RX: " + data);
            processCommand(data);
        }
      }
    }
};

void setupBLE() {
  NimBLEDevice::init("SuperBadge OS");
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    NIMBLE_PROPERTY::NOTIFY
                  );

  NimBLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                       NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
                     );

  pRxCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  Serial.println("BLE Advertising Started!");
}

void sendBLE(String message) {
    if (deviceConnected && pTxCharacteristic != NULL) {
        pTxCharacteristic->setValue(message.c_str());
        pTxCharacteristic->notify();
    }
}
