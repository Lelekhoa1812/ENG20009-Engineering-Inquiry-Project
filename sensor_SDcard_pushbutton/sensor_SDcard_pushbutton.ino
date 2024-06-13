#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <BH1750.h>
#include "RTClib.h" 
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_GFX.h>
#include <SdFat.h>

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}; //create a matrix for displaying the date
DateTime current;

SdFs sd;
FsFile file;

const int button1 = 2;
const int button2 = 3;
const int button3 = 4;
const int button4 = 5;

const uint8_t SD_CS_PIN = A3;
const uint8_t SOFT_MISO_PIN = 12;
const uint8_t SOFT_MOSI_PIN = 11;
const uint8_t SOFT_SCK_PIN  = 13;
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)

#define TFT_CS    10
#define TFT_RST   6 
#define TFT_DC    7 
#define TFT_SCLK 13   
#define TFT_MOSI 11   

//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

BH1750 lightMeter;
Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

void setup() {
  Serial.begin(9600);

  rtc.begin();    
  // Adjusting to Melbourne GMT+11 time.
  rtc.adjust(DateTime(2023,05,18,10,39,30)); 

  while (!Serial);
  Serial.println(F("BME680 test"));
  Serial.println(F("BH1750 Test begin"));

  if (!sd.begin(SD_CONFIG)) {
    Serial.println("SD card initialization failed!");
    sd.initErrorHalt();
    while (1);
  }

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  Serial.print(F("Hello! ST77xx TFT Test"));
  tft.initR(INITR_BLACKTAB);      
  tft.setRotation(3);
  Serial.println(F("Initialized"));
  tft.fillScreen(ST77XX_BLACK);

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  lightMeter.begin();

pinMode(button1, INPUT_PULLUP);
pinMode(button2, INPUT_PULLUP);
pinMode(button3, INPUT_PULLUP);
pinMode(button4, INPUT_PULLUP);

  tft.setCursor(5, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("B1: BH1750 sensor");
  tft.setCursor(5, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("B2: BME680 sensor");
  tft.setCursor(5, 50);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("B3: Both sensor");
  tft.setCursor(5, 70);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("B4: Save Data to SD card"); 

  if (!file.open("sensor_data.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("open failed"));
  }

  file.close(); //release file
}

String getAQData() {
bme.performReading();

  String data = "Temperature = " + String(bme.temperature) + " *C\n" +
                "Pressure = " + String(bme.pressure / 100.0) + " hPa\n" +
                "Humidity = " + String(bme.humidity) + " %\n" +
                "Gas = " + String(bme.gas_resistance / 1000.0) + " KOhms\n" +
                "Approx. Altitude = " + String(bme.readAltitude(SEALEVELPRESSURE_HPA)) + " m";

  return data;

}

void display(DateTime now){
  Serial.print(now.year(), DEC);// get the current year
  Serial.print('/');
  Serial.print(now.month(), DEC); //get the current month
  Serial.print('/');
  Serial.print(now.day(), DEC);//get the current day
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]); //get the date
  Serial.print(") ");
  Serial.print(now.hour(), DEC);//get current hour
  Serial.print(':');
  Serial.print(now.minute(), DEC); //get current minute
  Serial.print(':');
  Serial.print(now.second(), DEC); //get current second
  Serial.println();
}

void loop() {
//Button 1: Light BH1750 sensor
//Button 2: Air Quality BME680 sensor
//Button 3: Both sensor
//Button 4: Save sensor data into SD card

current = rtc.now();
display(current);
delay(1000);

  if (digitalRead(button1) == LOW) {
    Serial.println("Button 1 pressed - BH1750 Light sensor");
    Light();

    delay(2000);
  } else if (digitalRead(button2) == LOW) {
    Serial.println("Button 2 pressed - BME680 Air Quality sensor");
    AQ();

    delay(2000);
  } else if (digitalRead(button3) == LOW) {
    Serial.println("Button 3 pressed - Both sensors work continuously");
    Both();

  } else if (digitalRead(button4) == LOW) {
    Serial.println("Button 4 pressed - Saving Sensor Data into SD card");

  // Read sensor data from BH1750 and BME680
    String sensordata = "";
    float lightData = lightMeter.readLightLevel();
    String aqData = getAQData();

    // Store sensor data in sensordata variable
    sensordata = "Light: " + String(lightData) + " lx\n" + "Air Quality: " + aqData;

    // Save sensor data to SD card
    WriteSD(file, sensordata);

    Serial.println("Data saved into sensor_data.txt");
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(5, 50);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.println("Data saved into sensor_data.txt");
    delay(2000);
    }
  }


void WriteSD(FsFile& file, const String& sensordata) {
  Serial.println("---   Saving To File   ---");

  if (!file.open("sensor_data.txt", O_RDWR | O_CREAT | O_APPEND)) {
    sd.errorHalt(F("open failed"));
  }

  file.println(sensordata);

  file.close();
}


void ReadSD(File file) {
  Serial.println("--- Reading From file! ---");
  
  file.open("sensor_data.txt", O_RDWR);
  file.seek(0);                 //go to char 0
  file.close();
}

void AQ() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);

  tft.setCursor(5, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Temperature = ");
  tft.setCursor(100, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.temperature);
  tft.setCursor(130, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("*C");

  tft.setCursor(5, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Pressure = ");  
  tft.setCursor(70, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.pressure / 100.0);  
  tft.setCursor(100, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" hPa");  

  tft.setCursor(5, 50);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Humidity = ");
  tft.setCursor(70, 50);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.humidity);
  tft.setCursor(100, 50);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" %");

  tft.setCursor(5, 70);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Gas = ");
  tft.setCursor(70, 70);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.gas_resistance / 1000.0);
  tft.setCursor(100, 70);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" KOhms");

  tft.setCursor(5, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sea Level = ");
  tft.setCursor(90, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
  tft.setCursor(120, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" m");
}

void Light() {
float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  Serial.println();

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);

  tft.setCursor(5, 60);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Light = ");
  tft.setCursor(70, 60);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(lux);
  tft.setCursor(100, 60);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" lx");
}


void Both() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);

  tft.setCursor(5, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Temperature = ");
  tft.setCursor(100, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.temperature);
  tft.setCursor(130, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("*C");

  tft.setCursor(5, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Pressure = ");  
  tft.setCursor(70, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.pressure / 100.0);  
  tft.setCursor(100, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" hPa");  

  tft.setCursor(5, 50);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Humidity = ");
  tft.setCursor(70, 50);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.humidity);
  tft.setCursor(100, 50);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" %");

  tft.setCursor(5, 70);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Gas = ");
  tft.setCursor(70, 70);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.gas_resistance / 1000.0);
  tft.setCursor(100, 70);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" KOhms");

  tft.setCursor(5, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sea Level = ");
  tft.setCursor(90, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
  tft.setCursor(120, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" m");

float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  Serial.println();

  tft.setCursor(5, 110);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Light = ");
  tft.setCursor(70, 110);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(lux);
  tft.setCursor(100, 110);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" lx");
}

void resetFunc() {
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextSize(1);

  tft.setCursor(50, 60);
  tft.setTextColor(ST77XX_RED);
  tft.println("Reseting...");
}