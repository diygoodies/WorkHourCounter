// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"

RTC_DS1307 rtc;
uint8_t dateread[20];
uint8_t temp[5];
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

struct RTCDateTime
{
  uint8_t year;     ///< Year offset from 2000
  uint8_t month;    ///< Month 1-12
  uint8_t day;      ///< Day 1-31
  uint8_t hour;     ///< Hours 0-23
  uint8_t minute;   ///< Minutes 0-59
  uint8_t second;   ///< Seconds 0-59
  uint8_t duty;
  uint16_t ttcount;
};

RTCDateTime RDT;

void setup () {
  while (!Serial); // for Leonardo/Micro/Zero

  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2019, 9, 30, 15, 01, 0));
  }
}

void loop () {


  if ( Serial.available()) 
  {
    uint8_t b=Serial.available();
    //Serial.println(b);
    for (uint8_t i = 0; i<b; i++) 
    {
      dateread[i] = Serial.read();
      //Serial.print(dateread[i],HEX);
      //Serial.print(" ");
    }
    //Serial.println(" ");
    if (dateread[b]!=0x0A)
    {
      dateread[b]=0x0A;
      //Serial.println("0x0A");
    }
    Serial.flush();
    if (strstr((char*)dateread,"stt"))
    {
      RDT.year = atoi(dateread+4);
      RDT.month = atoi(dateread + 7);
      RDT.day = atoi(dateread + 10);
      RDT.hour = atoi(dateread + 13);
      RDT.minute = atoi(dateread + 16);
      RDT.second = atoi(dateread + 19);      
      Serial.println("Time set:");//stt 19-09-30 15:46:48
      Serial.print (RDT.year,DEC);
      Serial.print('-');
      Serial.print (RDT.month,DEC);
      Serial.print('-');
      Serial.print (RDT.day,DEC);
      Serial.print(' ');  
      Serial.print (RDT.hour,DEC);
      Serial.print(':');
      Serial.print (RDT.minute,DEC);
      Serial.print(':');
      Serial.println (RDT.second,DEC);

      rtc.adjust(DateTime(RDT.year, RDT.month, RDT.day, RDT.hour, RDT.minute, RDT.second));
    }
  }


    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    Serial.println();
/*
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
*/
    delay(3000);
}
