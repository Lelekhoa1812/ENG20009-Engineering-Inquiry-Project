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
// To use the hardware I2C (TWI) interface of the chipKit you must connect
// the pins as follows:
//
// chipKit Uno32/uC32:
// ----------------------
// DS1307:  SDA pin   -> Analog 4
//          SCL pin   -> Analog 5
// *** Please note that JP6 and JP8 must be in the I2C position (closest to the analog pins)
//
// chipKit Max32:
// ----------------------
// DS1307:  SDA pin   -> Digital 20 (the pin labeled SDA)
//          SCL pin   -> Digital 21 (the pin labeled SCL)
//
// The chipKit boards does not have pull-up resistors on the hardware I2C interface
// so external pull-up resistors on the data and clock signals are required.
//
// You can connect the DS1307 to any available pin but if you use any
// other than what is described above the library will fall back to
// a software-based, TWI-like protocol which will require exclusive access 
// to the pins used.
//

#include <DS1307.h>

DS1307_RAM ramBuffer;
DS1307 rtc(SDA, SCL); 

void setup()
{
  // Setup Serial connection
  Serial.begin(115200);

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
