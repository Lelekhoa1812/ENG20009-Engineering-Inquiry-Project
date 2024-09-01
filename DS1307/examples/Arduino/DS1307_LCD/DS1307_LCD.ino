// DS1307_LCD 
// Copyright (C)2016 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/
//
// A quick demo of how to use my DS1307-library to make a quick
// clock using a DS1307 and a 16x2 LCD.
//
// NOTE: This example has only been tested on an Arduino Uno. It may behave 
// incorrectly on other boards.
//
// Connect your LCD to the following pins:
// LCD:     DB7       -> Arduino Digital 6
//          DB6       -> Arduino Digital 7
//          DB5       -> Arduino Digital 8
//          DB4       -> Arduino Digital 9
//          E         -> Arduino Digital 10
//          RS        -> Arduino Digital 11
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


#include <LiquidCrystal.h>
#include <DS1307.h>

// Init the DS1307
DS1307 rtc(SDA, SCL);

// Init the LCD
LiquidCrystal lcd(11, 10, 9, 8, 7, 6);

void setup()
{
  // Initialize the rtc object
  rtc.begin();
  
  // Set the clock to run-mode
  rtc.halt(false);
  
  // Setup LCD to 16x2 characters
  lcd.begin(16, 2);

  // The following lines can be commented out to use the values already stored in the DS1307
  rtc.setDOW(SUNDAY);        // Set Day-of-Week to SUNDAY
  rtc.setTime(12, 0, 0);     // Set the time to 12:00:00 (24hr format)
  rtc.setDate(3, 10, 2010);   // Set the date to October 3th, 2010

  // Set SQW/Out rate to 1Hz, and enable SQW
  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);
}

void loop()
{
  // Display time centered on the upper line
  lcd.setCursor(4, 0);
  lcd.print(rtc.getTimeStr());
  
  // Display abbreviated Day-of-Week in the lower left corner
  lcd.setCursor(0, 1);
  lcd.print(rtc.getDOWStr(FORMAT_SHORT));
  
  // Display date in the lower right corner
  lcd.setCursor(6, 1);
  lcd.print(rtc.getDateStr());

  // Wait one second before repeating :)
  delay (1000);
}
