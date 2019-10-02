// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"
#include <Wire.h>
#define DUTY_3to1 6
#define DUTY_1to2 14
#define DUTY_2to3 22
RTC_DS1307 rtc;
uint8_t dateread[20];
uint8_t temp[10];
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const int mainpower=14;
const int gluepower=15;

uint8_t mainpowercnt=0;
uint8_t gluepowercnt=0;

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

RTCDateTime rdt,rdt1;
uint8_t minutecmp=0;
uint8_t dutycmp=0;
uint16_t eeprom_count=2;


void setup () {
  while (!Serial); // for Leonardo/Micro/Zero

  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  pinMode(mainpower, INPUT);
  pinMode(gluepower, INPUT);
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2019, 9, 30, 15, 01, 0));
  }
  //i2c_eeprom_read_buffer(0x50, 0, (byte*)&eeprom_count, sizeof(eeprom_count));
  read_struct();

  if (eeprom_count<2)
  {
   eeprom_count=2;  
  }
  Serial.print("eeprom_count ");
  Serial.println (eeprom_count);
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
      rdt.year = atoi(dateread+4);
      rdt.month = atoi(dateread + 7);
      rdt.day = atoi(dateread + 10);
      rdt.hour = atoi(dateread + 13);
      rdt.minute = atoi(dateread + 16);
      rdt.second = atoi(dateread + 19);      
      Serial.println("Time set:");//stt 19-09-30 15:46:48
      Serial.print (rdt.year,DEC);
      Serial.print('-');
      Serial.print (rdt.month,DEC);
      Serial.print('-');
      Serial.print (rdt.day,DEC);
      Serial.print(' ');  
      Serial.print (rdt.hour,DEC);
      Serial.print(':');
      Serial.print (rdt.minute,DEC);
      Serial.print(':');
      Serial.println (rdt.second,DEC);

      rtc.adjust(DateTime(rdt.year, rdt.month, rdt.day, rdt.hour, rdt.minute, rdt.second));
    }

    if (strstr((char*)dateread,"rst"))
    {
      Serial.println("Reset memory");
      eeprom_count=2;
      //i2c_eeprom_write_page(0x50, 0, (byte*)&eeprom_count, sizeof(eeprom_count));
      delay(10);
      i2c_eeprom_write_byte(0x50, 0, eeprom_count>>8);
      delay(10);
      i2c_eeprom_write_byte(0x50, 1, eeprom_count);
    }
   
    if (strstr((char*)dateread,"tst"))
    {
      write_struct();
    }

    if (strstr((char*)dateread,"rdm"))
    {
      read_struct();
    }  
  }


    DateTime now = rtc.now();

    rdt.year=now.year()-2000;
    rdt.month=now.month();
    rdt.day=now.day();
    rdt.hour=now.hour();
    rdt.minute=now.minute();
    rdt.minute=now.minute();
    rdt.second=now.second();
    
      Serial.print (rdt.year,DEC);
      Serial.print('-');
      Serial.print (rdt.month,DEC);
      Serial.print('-');
      Serial.print (rdt.day,DEC);
      Serial.print(' ');  
      Serial.print (rdt.hour,DEC);
      Serial.print(':');
      Serial.print (rdt.minute,DEC);
      Serial.print(':');
      Serial.print (rdt.second,DEC);
      Serial.print(" Duty:");
      Serial.print(rdt.duty&0x0F,DEC);
      Serial.print(" Mode:");
      Serial.print(rdt.duty>>4,HEX);
      Serial.print(" Count:");
      Serial.println(rdt.ttcount,DEC);
      
    if (digitalRead(mainpower) == HIGH)
    {
      mainpowercnt++;
      if (mainpowercnt>5)
      {
        //Serial.println("mainpowercnt 1");
        if (minutecmp!=rdt.minute)
        {
          minutecmp=rdt.minute;
          rdt.ttcount++;          
        }
        mainpowercnt=0;
        rdt.duty=0x10;
      }
    }
    else
    {
      //Serial.println("mainpowercnt 0");
      mainpowercnt=0;
      rdt.duty=0x00;
    }
    
    if (digitalRead(gluepower) == HIGH)
    {
      gluepowercnt++;
      if (gluepowercnt>5)
      {
        //Serial.println("gluepowercnt 1");
        gluepowercnt=0;
        rdt.duty|=0x20;
      }
    }
    else
    {
      //Serial.println("gluepowercnt 0");
      gluepowercnt=0;
    }

    if ((DUTY_3to1<=rdt.hour)&&(rdt.hour<DUTY_1to2))
    {
      rdt.duty|=0x01;
    }
    else if ((DUTY_1to2<=rdt.hour)&&(rdt.hour<DUTY_2to3))
    {
      rdt.duty|=0x02;
    }
    else if ((DUTY_2to3<=rdt.hour)&&(rdt.hour<DUTY_3to1))
    {
      rdt.duty|=0x03;
    }

    if (rdt.duty!=dutycmp)
    {
      dutycmp=rdt.duty;
      write_struct();
    }
     
    delay(5000);
}
void read_struct(void)
{
     Serial.println("Read struct");

     //i2c_eeprom_read_buffer(0x50, 0, (byte*)&eeprom_count, sizeof(eeprom_count));

     delay(10);
     eeprom_count=i2c_eeprom_read_byte(0x50, 0)<<8;;
     delay(10);
     eeprom_count=i2c_eeprom_read_byte(0x50, 1);

     Serial.print("eeprom_count ");
     Serial.println (eeprom_count);
     
     uint16_t ri=2;
     while (ri<eeprom_count)
     {
      delay(100);
     rdt1.year=i2c_eeprom_read_byte(0x50, ri);
     delay(10);
     rdt1.month=i2c_eeprom_read_byte(0x50, ri+1);
     delay(10);
     rdt1.day=i2c_eeprom_read_byte(0x50, ri+2);
     delay(10);
     rdt1.hour=i2c_eeprom_read_byte(0x50, ri+3);
     delay(10);
     rdt1.minute=i2c_eeprom_read_byte(0x50, ri+4);
     delay(10);
     rdt1.second=i2c_eeprom_read_byte(0x50, ri+5);
     delay(10);
     rdt1.duty=i2c_eeprom_read_byte(0x50, ri+6);
     delay(10);
     rdt1.ttcount=i2c_eeprom_read_byte(0x50, ri+7)<<8;
     delay(10);
     rdt1.ttcount|=i2c_eeprom_read_byte(0x50, ri+8);

      //i2c_eeprom_read_buffer(0x50, ri, (byte*)&rdt1, sizeof(rdt1));

      Serial.print(rdt1.year,DEC);
      Serial.print('-');
      Serial.print(rdt1.month,DEC);
      Serial.print('-');
      Serial.print(rdt1.day,DEC);
      Serial.print(' ');  
      Serial.print(rdt1.hour,DEC);
      Serial.print(':');
      Serial.print(rdt1.minute,DEC);
      Serial.print(':');
      Serial.print(rdt1.second,DEC);
      Serial.print(" Duty:");
      Serial.print(rdt1.duty&0x0F,DEC);
      Serial.print(" Mode:");
      Serial.print(rdt1.duty>>4,HEX);
      Serial.print(" Count:");
      Serial.println(rdt1.ttcount,DEC);

      if (rdt.ttcount<rdt1.ttcount)
      {
        rdt.ttcount=rdt1.ttcount;
      }
   
      Serial.print("eeprom_count ");
      Serial.println (ri);
      Serial.println("");
      ri=ri+9;
     }  
}

void write_struct(void)
{
  uint8_t tr=0;
  while(1)
  {   
     Serial.println("Write struct");
     
     delay(10);
     i2c_eeprom_write_byte(0x50, eeprom_count, rdt.year);
     delay(10);
     i2c_eeprom_write_byte(0x50, eeprom_count+1, rdt.month);
     delay(10);
     i2c_eeprom_write_byte(0x50, eeprom_count+2, rdt.day);
     delay(10);
     i2c_eeprom_write_byte(0x50, eeprom_count+3, rdt.hour);
     delay(10);
     i2c_eeprom_write_byte(0x50, eeprom_count+4, rdt.minute);
     delay(10);
     i2c_eeprom_write_byte(0x50, eeprom_count+5, rdt.second);
     delay(10);
     i2c_eeprom_write_byte(0x50, eeprom_count+6, rdt.duty);
     delay(10);
     i2c_eeprom_write_byte(0x50, eeprom_count+7, rdt.ttcount>>8);
     delay(10);
     i2c_eeprom_write_byte(0x50, eeprom_count+8, rdt.ttcount&0x0F);
     //i2c_eeprom_write_page(0x50, eeprom_count+7, (byte*)&rdt.ttcount, sizeof(rdt.ttcount));

     //i2c_eeprom_write_page(0x50, eeprom_count, (byte*)&rdt, sizeof(rdt));
   
     delay(300);
     Serial.println("Read struct");
     rdt1.year=i2c_eeprom_read_byte(0x50, eeprom_count);
     delay(10);
     rdt1.month=i2c_eeprom_read_byte(0x50, eeprom_count+1);
     delay(10);
     rdt1.day=i2c_eeprom_read_byte(0x50, eeprom_count+2);
     delay(10);
     rdt1.hour=i2c_eeprom_read_byte(0x50, eeprom_count+3);
     delay(10);
     rdt1.minute=i2c_eeprom_read_byte(0x50, eeprom_count+4);
     delay(10);
     rdt1.second=i2c_eeprom_read_byte(0x50, eeprom_count+5);
     delay(10);
     rdt1.duty=i2c_eeprom_read_byte(0x50, eeprom_count+6);
     delay(10);
     rdt1.ttcount=i2c_eeprom_read_byte(0x50, eeprom_count+7)<<8;
     delay(10);
     rdt1.ttcount|=i2c_eeprom_read_byte(0x50, eeprom_count+8);
     //i2c_eeprom_read_buffer(0x50, eeprom_count+7, (byte*)&rdt1.ttcount, sizeof(rdt1.ttcount));
     
     //i2c_eeprom_read_buffer(0x50, eeprom_count, (byte*)&rdt1, sizeof(rdt1));
     
      Serial.print(rdt1.year,DEC);
      Serial.print('-');
      Serial.print(rdt1.month,DEC);
      Serial.print('-');
      Serial.print(rdt1.day,DEC);
      Serial.print(' ');  
      Serial.print(rdt1.hour,DEC);
      Serial.print(':');
      Serial.print(rdt1.minute,DEC);
      Serial.print(':');
      Serial.print(rdt1.second,DEC);
      Serial.print(" Duty:");
      Serial.print(rdt1.duty&0x0F,DEC);
      Serial.print(" Mode:");
      Serial.print(rdt1.duty>>4,HEX);
      Serial.print(" Count:");
      Serial.println(rdt1.ttcount,DEC);

      tr++;

     if (((rdt1.year==rdt.year)&&(rdt1.month==rdt.month)&&(rdt1.day==rdt.day)&&(rdt1.hour==rdt.hour)&&(rdt1.minute==rdt.minute)&&(rdt1.second==rdt.second)&&(rdt1.duty==rdt.duty)&&(rdt1.ttcount==rdt.ttcount))||(tr>5))
     {
      Serial.println("Verify OK");
      delay(10);
      eeprom_count=eeprom_count+9;
      //i2c_eeprom_write_page(0x50, 0, (byte*)&eeprom_count, sizeof(eeprom_count));
      i2c_eeprom_write_byte(0x50, 0, eeprom_count>>8);
      delay(10);
      i2c_eeprom_write_byte(0x50, 1, eeprom_count);
      delay(300);


      eeprom_count=i2c_eeprom_read_byte(0x50, 0)<<8;;
      delay(10);
      eeprom_count=i2c_eeprom_read_byte(0x50, 1);
      //i2c_eeprom_read_buffer(0x50, 0, (byte*)&eeprom_count, sizeof(eeprom_count));
      Serial.print("eeprom_count ");
      Serial.println (eeprom_count);
      break;
     } 
  } 
}

// Library calls for I2C memory below

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
    int rdata = data;
    //Wire.setClock(100000); 
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(rdata);
    Wire.endTransmission();
  }

  // WARNING: address is a page address, 6-bit end will wrap around
  // also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
  void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
    //Wire.setClock(100000); 
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddresspage >> 8)); // MSB
    Wire.write((int)(eeaddresspage & 0xFF)); // LSB
    byte c;
    for ( c = 0; c < length; c++)
      Wire.write(data[c]);
    Wire.endTransmission();
  }

  byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
    byte rdata = 0xFF;
    //Wire.setClock(100000); 
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()) rdata = Wire.read();
    return rdata;
  }

  // maybe let's not read more than 30 or 32 bytes at a time!
  void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
    //Wire.setClock(10000); 
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,length);
    int c = 0;
    for ( c = 0; c < length; c++ )
      if (Wire.available()) buffer[c] = Wire.read();
  }
