// DS1307_RAM_Demo 
// Copyright (C)2016 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/
//
// A quick demo of how to use my DS1307-library to 
// read and write to the internal RAM of the DS1307.
// Both burst (all 56 bytes at once) and single byte
// reads and write is demonstrated.
// All output is sent to the serial-port at 115200 baud.
//
// To use the hardware I2C (TWI) interface of the Arduino you must connect
// the pins as follows:
//
// Arduino Uno/2009:
// ----------------------
// DS1307:  SDA pin   -> Arduino Analog 4 or the dedicated SDA pin
//          SCL pin   -> Arduino Analog 5 or the dedicated SCL pin
//
// Arduino Leonardo:
// ----------------------
// DS1307:  SDA pin   -> Arduino Digital 2 or the dedicated SDA pin
//          SCL pin   -> Arduino Digital 3 or the dedicated SCL pin
//
// Arduino Mega:
// ----------------------
// DS1307:  SDA pin   -> Arduino Digital 20 (SDA) or the dedicated SDA pin
//          SCL pin   -> Arduino Digital 21 (SCL) or the dedicated SCL pin
//
// Arduino Due:
// ----------------------
// DS1307:  SDA pin   -> Arduino Digital 20 (SDA) or the dedicated SDA1 (Digital 70) pin
//          SCL pin   -> Arduino Digital 21 (SCL) or the dedicated SCL1 (Digital 71) pin
//
// The internal pull-up resistors will be activated when using the 
// hardware I2C interfaces.
//
// You can connect the DS1307 to any available pin but if you use any
// other than what is described above the library will fall back to
// a software-based, TWI-like protocol which will require exclusive access 
// to the pins used, and you will also have to use appropriate, external
// pull-up resistors on the data and clock signals.
//

#include <DS1307.h>

DS1307_RAM ramBuffer;
DS1307 rtc(SDA, SCL); 

void setup()
{
  // Setup Serial connection
  Serial.begin(115200);
  // Uncomment the next line if you are using an Arduino Leonardo
  //while (!Serial) {}

  // Initialize the rtc object
  rtc.begin();
}

void bufferDump(char st[])
{
  Serial.write(st);
  Serial.println("");
  for (int i=0; i<56; i++)
  {
    Serial.print("0x");
    Serial.print(ramBuffer.cell[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
  Serial.println("--------------------------------------------------------");
}

void comment(char st[])
{
  Serial.println("");
  Serial.print("---> ");
  Serial.write(st);
  Serial.println("");
  Serial.println("");
}

void loop()
{
  Serial.println("");
  bufferDump("Initial buffer");

  comment("Filling buffer with data...");
  for (int i=0; i<56; i++)
    ramBuffer.cell[i]=i;

  comment("Writing buffer to RAM...");
  rtc.writeBuffer(ramBuffer);
  bufferDump("Buffer written to RAM...");
  
  comment("Clearing buffer...");
  for (int i=0; i<56; i++)
    ramBuffer.cell[i]=0;
  bufferDump("Cleared buffer...");
  
  comment("Setting byte 15 (0x0F) to value 160 (0xA0)...");
  rtc.poke(15,160);
  
  comment("Reading buffer from RAM...");
  ramBuffer=rtc.readBuffer();
  bufferDump("Buffer read from RAM...");

  int temp;
  comment("Reading address 18 (0x12). This should return 18, 0x12.");
  temp = rtc.peek(18);
  Serial.print("Return value: ");
  Serial.print(temp, DEC);
  Serial.print(", 0x");
  Serial.println(temp, HEX);


  Serial.println("");
  Serial.println("");
  Serial.println("***** End of demo *****");
 
  while (1){};
}
