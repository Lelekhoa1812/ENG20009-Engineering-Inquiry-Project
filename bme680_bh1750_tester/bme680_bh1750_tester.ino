#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <BH1750.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_GFX.h>

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

  while (!Serial);
  Serial.println(F("BME680 test"));
  Serial.println(F("BH1750 Test begin"));


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
Serial.println("Choosing your sensor:\n1 - Light sensor?\n2 - Temperature, Pressure and Humidity sensor?\n5 - Continous displaying two sensors");
}

void loop() {
Light();
Pressure();
delay(1000);
}


void Pressure(){
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
  tft.setCursor(100, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
  tft.setCursor(130, 90);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(" hPa");
}

void Light() {
float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  Serial.println();


  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);

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