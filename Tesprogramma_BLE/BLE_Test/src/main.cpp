#include <Arduino.h>
#include <ArduinoBLE.h>

int state = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial); //Waits for the serial port to open

  // initialize the Bluetooth® Low Energy hardware
  BLE.begin(); 
  
  // start scanning for peripherals
  // BLE.scanForUuid(""); 


  /*
  BLEService ledService(" any 128-bit UUID "); //
  moet overeenkomen met peripheral
  */

}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    // bescherming tegen ontvangen andere truck als werken met Uuid
     if (peripheral.localName() != "") {
      return;
    }
    
    // stop scanning
    BLE.stopScan();

    doeIets(peripheral);



  }





}


void doeIets(BLEDevice peripheral){
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }


  BLECharacteristic testCharacteristic = peripheral.characteristic();

  /*
  BLEByteCharacteristic naam("", BLERead | BLEWrite);
  bepaald door peripheral
  */
  if (!testCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    return;

  }

  // ledCharacteristic.writeValue();
  // ledCharacteristic.readValue()

}