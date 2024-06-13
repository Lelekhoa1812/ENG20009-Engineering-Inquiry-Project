#include "RTClib.h" 

RTC_DS1307 rtc;

//char daysOfTheWeek[7][12] = {"Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

void setup () {
  Serial.begin(9600);

  rtc.begin();

  rtc.adjust(DateTime(2023, 5, 22, 3, 30, 0));
  // This line sets the RTC with an explicit date & time, for example to set
  // March 21, 2023 at 9am you would call:
  // rtc.adjust(DateTime(2023, 3, 21, 9, 0, 0));

  //use the  "rtc.isrunning()" function to determine if the RTC is running, 
  //if the function returns true then the time has already been configured on the breakout board. 
}

void loop () {
  //update "now" variable
    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    //Serial.print(" (");
    //Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    //Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    delay(1000);
}