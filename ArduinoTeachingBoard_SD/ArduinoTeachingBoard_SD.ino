#include "SdFat.h"

//define file system for sd card
SdFs sd;
FsFile file;
//^ configuration for FAT16/FAT32 and exFAT.

// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = A3;
//
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 12;
const uint8_t SOFT_MOSI_PIN = 11;
const uint8_t SOFT_SCK_PIN  = 13;

// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)

void setup() {
  Serial.begin(9600);
  
  if (!sd.begin(SD_CONFIG)) {
    Serial.println("SD card initialization failed!");
    sd.initErrorHalt();
    while (1);
  }

  // Open/create a file for writing
  if (!file.open("message.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("open failed"));
  }

  file.close(); //release file

  Serial.println("Please enter a message and press enter:");
}

void loop() {
  if (Serial.available()) {

    // Read the incoming message
    String message = Serial.readString();
    
    WriteSD(file, message);
    
    ReadSD(file);
  }
}

void WriteSD(File file, String message) {  
  Serial.println("---   Saving To File   ---");

  file.open("message.txt", O_RDWR);
  file.rewind();                //Go to file position 0
  file.println("Message written using SoftwareSPI");
  file.println(message);
  file.close();
}

void ReadSD(File file) {
  Serial.println("--- Reading From file! ---");
  
  file.open("message.txt", O_RDWR);
  file.seek(0);                 //go to char 0
  String contents = file.readString();
  Serial.println(contents);
  file.close();
}