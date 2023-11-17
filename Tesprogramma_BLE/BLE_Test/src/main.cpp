#include <Arduino.h>
#include <ArduinoBLE.h>

int state = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial); //Waits for the serial port to open

  // initialize the Bluetooth® Low Energy hardware
  BLE.begin(); 
  
  // start scanning for peripherals
  BLE.scanForUuid("550e8400-e29b-41d4-a716-446655440000"); 


  /*
  BLEService testService(" any 128-bit UUID "); //
  defined in peripheral -> 128-bit UUID moet ingevuld worden hierboven 
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

    // dubbel-check correct peripheral
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


  BLECharacteristic testCharacteristic = peripheral.characteristic("");
  // in central (moet overeenkomen met peripheral definition)


  // BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
  // BLEByteCharacteristic naam("", BLERead | BLEWrite);
  // vb van characteristic defined in peripheral 

 
  if (!testCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    return;

  }

  // ledCharacteristic.writeValue();
  // ledCharacteristic.readValue()

}