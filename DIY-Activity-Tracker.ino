// Define colors
#define BLACK           0x00
#define BLUE            0xE0
#define RED             0x03
#define GREEN           0x1C
#define DGREEN          0x0C
#define YELLOW          0x1F
#define WHITE           0xFF
#define ALPHA           0xFE
#define BROWN           0x32

// Include necessary libraries
#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include "BMA250.h"
#include "DSRTCLib.h"


// Accel Setup
BMA250 accel;
int oldXval;
int oldYval; 
int oldZval;

// Screen Setup 
TinyScreen display = TinyScreen(TinyScreenPlus);
int iCounter = 0;

// Logger Setup
const int chipSelect = 10;
           
// RTC Setup
int ledPin =  13;    // LED connected to digital pin 13
int INT_PIN = 3;     // INTerrupt pin from the RTC. On Arduino Uno, this should be mapped to digital pin 2 or pin 3, which support external interrupts
int int_number = 1;  // On Arduino Uno, INT0 corresponds to pin 2, and INT1 to pin 3
DS1339 RTC = DS1339();


void setup(void) {
    
  pinMode(ledPin, OUTPUT);    
  digitalWrite(ledPin, LOW);
  Serial.begin(9600);
  Wire.begin();
  display.begin();

  // Initialize Accelerometer
  accel.begin(BMA250_range_2g, BMA250_update_time_64ms);                                       
 
  // Initialize SD
  Serial.print("Initializing SD card...");

  // Check if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  // Open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  
  if (dataFile){
    dataFile.println("......................");
    dataFile.println("Date, Time, X, Y, Z, Temp(C)");
    dataFile.close();  
  }
  else 
  {
    Serial.println("Couldn't open data file");
    return;
  }

  Serial.println ("DSRTCLib Tests");

  // Initialize RTC
  RTC.start(); 
  //set_time();  //**COMMENT OUT THIS LINE AFTER YOU SET THE TIME**//
        
}


int read_int(char sep)
{
  static byte c;
  static int i;

  i = 0;
  while (1)
  {
    while (!Serial.available())
    {;}
 
    c = Serial.read();
                       
  
    if (c == sep)
    {
                                         
                           
      return i;
    }
    if (isdigit(c))
    {
      i = i * 10 + c - '0';
    }
    else
    {
      Serial.print("\r\nERROR: \"");
      Serial.write(c);
      Serial.print("\" is not a digit\r\n");
      return -1;
    }
  }
}


int read_int(int numbytes)
{
  static byte c;
  static int i;
  int num = 0;

  i = 0;
  while (1)
  {
    while (!Serial.available())
    {;}
 
    c = Serial.read();
    num++;
                       
  
    if (isdigit(c))
    {
      i = i * 10 + c - '0';
    }
    else
    {
      Serial.print("\r\nERROR: \"");
      Serial.write(c);
      Serial.print("\" is not a digit\r\n");
      return -1;
    }
    if (num == numbytes)
    {
                                         
                           
      return i;
    }
  }
}


int read_date(int *year, int *month, int *day, int *hour, int* minute, int* second)
{

  *year = read_int(4);
  *month = read_int(2);
  *day = read_int(' ');
  *hour = read_int(':');
  *minute = read_int(':');
  *second = read_int(2);

  return 0;
}


void currentTime(String &nowDate, String &nowTime){
   RTC.readTime();
   nowDate = String(int(RTC.getMonths())) + "/" + String (int(RTC.getDays())) + "/" + String(RTC.getYears()-2000);
   nowTime = String(int(RTC.getHours())) + ":" + String(int(RTC.getMinutes())) + ":" + String(int(RTC.getSeconds()));  
}  


void loop() {

  // Getting date and time
  String ahoraDate;
  String ahoraTime;
  currentTime(ahoraDate, ahoraTime);
  String ahora = ahoraDate + "  " + ahoraTime;

  // Updating display
  display.setFont(liberationSans_8ptFontInfo);
  display.setCursor(0,0);
  display.fontColor(YELLOW,BLACK);
  display.print(ahora);
  // Reading accelerometer data so it can be used to draw the graph 
  // and be logged to the file
  accel.read();                                                    
  drawGraph();

  // Logging
  String dataString = "";
  dataString = String(ahoraDate)  + ", " 
             + String(ahoraTime)  + ", " 
             + String(accel.X) + ", " 
             + String(accel.Y) + ", " 
             + String(accel.Z) + ", " 
             + String((accel.rawTemp*0.5)+24.0,1);
  

  // If the file is available, write to it:
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if(dataFile)
  {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  }
  else
  {
    Serial.println("Couldn't access file");
  }
  delay(200);

                                   
}

void drawGraph() {

    // Getting data from accelerometer and transform it
    int xVal = accel.X;
    xVal = map(xVal, -500, 500, 12,54);
    int yVal = accel.Y;
    yVal = map(yVal, -500, 500, 12,54);
    int zVal = accel.Z;
    zVal = map(zVal, -500, 500, 12,54);
    
    // Drawing general display structure
    display.drawLine(0,35,96,35, WHITE);
    display.drawLine(0,10,96,10, WHITE);
    display.drawLine(96,10,96,60, WHITE);
    display.drawLine(0,60,96,60, WHITE);
    display.drawLine(0,10,0,60, WHITE);

    // Reseting the screen, if counter equals 0
    if(iCounter ==0){
      display.clearWindow(1,11,94,54);
      iCounter++;
    }
    // Drawing new data points and line of accelerometer
    else if (iCounter<95){
      display.drawPixel(iCounter,xVal, YELLOW);
      display.drawPixel(iCounter,yVal, GREEN);
      display.drawPixel(iCounter,zVal, RED);
      display.drawLine((iCounter-1),oldXval, iCounter, xVal, YELLOW);
      display.drawLine((iCounter-1),oldYval, iCounter, yVal, GREEN); 
      display.drawLine((iCounter-1),oldZval, iCounter, zVal, RED);  
      iCounter++;
      oldXval = xVal;
      oldYval = yVal;
      oldZval = zVal;
    }
    // Resetting counter
    else{
      iCounter = 0;
    }
}




void set_time()
{
    Serial.println("Enter date and time (YYYYMMDD HH:MM:SS)");
    int year, month, day, hour, minute, second;
    int result = read_date(&year, &month, &day, &hour, &minute, &second);
    if (result != 0) {
      Serial.println("Date not in correct format!");
      return;
    } 
    
                             
    RTC.setSeconds(second);
    RTC.setMinutes(minute);
    RTC.setHours(hour);
    RTC.setDays(day);
    RTC.setMonths(month);
    RTC.setYears(year);
    RTC.writeTime();
    read_time();
}

void read_time() 
{
  Serial.print ("The current time is ");
  RTC.readTime();                                          
  printTime(0);
  Serial.println();
  
}


void printTime(byte type)
{
                                                          
  if(!type)
  {
    Serial.print(int(RTC.getMonths()));
    Serial.print("/");  
    Serial.print(int(RTC.getDays()));
    Serial.print("/");  
    Serial.print(RTC.getYears());
  }
  else
  {
                                                                                                                 
    {
      Serial.print(int(RTC.getDayOfWeek()));
      Serial.print("th day of week, ");
    }
          
    {
      Serial.print(int(RTC.getDays()));
      Serial.print("th day of month, ");      
    }
  }
  
  Serial.print("  ");
  Serial.print(int(RTC.getHours()));
  Serial.print(":");
  Serial.print(int(RTC.getMinutes()));
  Serial.print(":");
  Serial.print(int(RTC.getSeconds()));  
}


