#include <LiquidCrystal_I2C.h>

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
LiquidCrystal_I2C lcd(0x27,16,2);
void setup() {
   lcd.init(); 
  lcd.backlight();
  Serial.begin(9600);
  while (!Serial) ; // wait for serial
  delay(200);
  lcd.setCursor(0,0);
  lcd.print("INITIALIZING...");
  delay(2000);
  lcd.clear();
  Serial.println("DS1307RTC Read Test");
  Serial.println("-------------------");
}

void loop() {
  tmElements_t tm;
lcd.setCursor(0,0);
  if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    
    lcd.setCursor(0,0);
    lcd.print("Time:");
    lcd.setCursor(6,0);
    print2digits(tm.Hour);
    lcd.print(tm.Hour);
    Serial.write(':');
    lcd.print(':');
   
    
    print2digits(tm.Minute);
    lcd.print(tm.Minute);
    Serial.write(':');
    lcd.print(':');

    print2digits(tm.Second);
    lcd.print(tm.Second);
    
    lcd.setCursor(0,1);
    lcd.print("Date:");
    lcd.setCursor(6,1);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    lcd.print(tm.Day);
    Serial.write('/');
    lcd.print('/');
    
    Serial.print(tm.Month);
    lcd.print(tm.Month);
    Serial.write('/');
    lcd.print('/');
    
    Serial.print(tmYearToCalendar(tm.Year));
    lcd.print(tmYearToCalendar(tm.Year));
    Serial.println();
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(9000);
  }
  delay(1000);
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
