/* CONFIGURATION_SETTINGS */

#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <BH1750.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_GFX.h>
#include <SdFat.h>
#include <RTClib.h>


//Define RTC DS13307
RTC_DS1307 rtc;
//char daysOfTheWeek[7][12] = {"Saturday", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};

SdFs sd;
FsFile file;

//Define button pins
#define BUTTON_PIN_1 2  // Pin for button 1
#define BUTTON_PIN_2 3  // Pin for button 2
#define DEBOUNCE_DELAY 50  // Debounce delay in milliseconds

// Define button states
#define BUTTON_IDLE    0
#define BUTTON_PRESSED 1
#define BUTTON_RELEASED 2

// Define sensor states
#define SENSOR_LIGHT    1
#define SENSOR_PRESSURE 2
#define SENSOR_BOTH 3

int buttonState1 = BUTTON_IDLE;
int buttonState2 = BUTTON_IDLE;
int buttonState3 = BUTTON_IDLE;

const int BUTTON_PIN_3 = 4;
const int button4 = 5;
const int SD_CS = 9;

//Define SD card 
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


//BME680 Setup
Adafruit_BME680 bme;

//BH1750 Setup
BH1750 lightMeter(0x23);

//SDI-12 Setup
#define DIRO 7
String command;
int deviceAddress = 0;
String deviceIdentification = "allccccccccmmmmmmvvvxxx";

// Sensor variables
int currentSensor = SENSOR_BOTH;
unsigned long lastSensorUpdateTime = 0;
const unsigned long sensorUpdateInterval = 3000;

//Define interrupt
volatile bool isInterrupted = false;

/* FUNCTIONS */

void setup() {
  //Arduino IDE Serial Monitor
  Serial.begin(9600);

  // ================ INTERRUPTION SETUP ================
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN_3), Interruption, FALLING);

  // ================ RTC DS1307 ================
  rtc.begin();
  rtc.adjust(DateTime(2023, 5, 23, 11, 0, 0));

  // ================ BME680 ================
  bme.begin(0x76);
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // ================ BH1750 ================
Wire.begin();
lightMeter.begin();

  // ================ Pushbuttons ================
pinMode(BUTTON_PIN_1, INPUT_PULLUP); //1
pinMode(BUTTON_PIN_2, INPUT_PULLUP); //2
pinMode(button4, INPUT_PULLUP);      //4

  Serial.print(F("Hello! ST77xx TFT Test"));
  tft.initR(INITR_BLACKTAB);      
  tft.setRotation(3);
  Serial.println(F("Initialized"));
  tft.fillScreen(ST77XX_BLACK);
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

  // ================ SD Card ================
  if (!sd.begin(SD_CONFIG)) {
    Serial.println("SD card initialization failed!");
    sd.initErrorHalt();
    while (1);
  }
  if (!file.open("sensor_data.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("open failed"));
  }

  file.close(); //release file

  // ================ SDI-12 ================
  Serial1.begin(1200, SERIAL_7E1);  //SDI-12 UART, configures serial port for 7 data bits, even parity, and 1 stop bit
  pinMode(DIRO, OUTPUT);               //DIRO Pin
  //HIGH to Receive from SDI-12
  digitalWrite(DIRO, HIGH);
}

void loop() {
  // ================ SDI12 menu ================
  int byte;
  if(Serial1.available()) {
    byte = Serial1.read();        //Reads incoming communication in bytes
    //Serial.println(byte);
    if (byte == 33) {             //If byte is command terminator (!)
      SDI12Receive(command);
      command = "";               //reset command string
    } else {
      if (byte != 0) {            //do not add start bit (0)
      command += char(byte);      //append byte to command string
      }
    }
  }
  // ================ Button menu ================
  ButtonMenu();
}

/* SEMI-FUNCTIONS */

//Create button menu
void ButtonMenu() {
  // Update button states
  updateButtonState(BUTTON_PIN_1, buttonState1);
  updateButtonState(BUTTON_PIN_2, buttonState2);

  if (buttonState1 == BUTTON_PRESSED) {
    Serial.println("Button 1 pressed - BH1750 Light sensor");
    currentSensor = SENSOR_LIGHT;
    displaySensor(currentSensor);

    delay(3000);
    resetFunc();
  } else if (buttonState2 == BUTTON_PRESSED) {
    Serial.println("Button 2 pressed - BME680 Air Quality sensor");
    currentSensor = SENSOR_PRESSURE;
    displaySensor(currentSensor);

    delay(2000);
    resetFunc();
  } else if (digitalRead(button4) == LOW) {
    Serial.println("Button 4 pressed - Saving Sensor Data into SD card");
    SaveSensorData();
    }
}

//Receive SDI12 command
void SDI12Receive(String input) {
  if ((String(input.charAt(0)) == "?") ) {
    SDI12Send(String("0"));
    Serial.println("Address = 0");
  }
  if ((String(input.charAt(0)) == "S") && (String(input.charAt(1)) == "A") && (String(input.charAt(2)) == "V") && (String(input.charAt(3)) == "E") ) {
    Serial.println("Save sensor data");
    SDI12Send(String("Save sensor data"));
    SaveSensorData();
  }
    if ((String(input.charAt(0)) == "T") && (String(input.charAt(1)) == "I") && (String(input.charAt(2)) == "M") && (String(input.charAt(3)) == "E") ) {
    Serial.println("Display time");
    SDI12Send(String("Display time"));
    Time();
    SaveSensorData();
  }

  //convert device address to string
  String address = String(deviceAddress);
  if (String(input.charAt(0)) == address) {  
    if ((String(input.charAt(1)) == "O") && (String(input.charAt(2)) == "D")&& (String(input.charAt(3)) == "1") ) {
      Light();
      BH1750toSDI12();
      Serial.println("Responding to OD1 command");
    }
     if ((String(input.charAt(1)) == "O") && (String(input.charAt(2)) == "D")&& (String(input.charAt(3)) == "2") ) {
      AQ();
      BME680toSDI12();
      Serial.println("Responding to OD2 command");
    } 
     if ((String(input.charAt(1)) == "O") && (String(input.charAt(2)) == "R")&& (String(input.charAt(3)) == "0") ) {
      currentSensor = SENSOR_BOTH;
      displayBothSensors();
      Serial.println("Responding to OR0 command");
    }  
     if ((String(input.charAt(1)) == "O") && (String(input.charAt(2)) == "A")&& (String(input.charAt(3)) == "1") ) {
      SDI12Send(String("1"));
      Serial.println("New Adress = 1");
    }  
     if ((String(input.charAt(1)) == "O") && (String(input.charAt(2)) == "M") ) {
      Serial.println("00036");
      SDI12Send(String("00036"));
    }
  }
}

//Send data to SDI12 and display
void SDI12Send(String message) {
  Serial.print("sensor value: "); Serial.println(message);
  
  digitalWrite(DIRO, LOW);
  delay(100);
  Serial1.print(message + String("\r\n"));
  Serial1.flush();    //wait for print to finish
  Serial1.end();
  Serial1.begin(1200, SERIAL_7E1);
  digitalWrite(DIRO, HIGH);
  //secondruntoken = 0;
}

//Read SD card
void ReadSD(File file) {
  Serial.println("--- Reading From file! ---");
  
  file.open("sensor_data.txt", O_RDWR);
  file.seek(0);                 //go to char 0
  file.close();
}

//Write SD card
void WriteSD(FsFile& file, const String& sensordata) {
  Serial.println("---   Saving To File   ---");

  if (!file.open("sensor_data.txt", O_RDWR | O_CREAT | O_APPEND)) {
    sd.errorHalt(F("open failed"));
  }

  file.println(sensordata);
  file.close();
}

/* CIRCUITRY-CONFIGURATIONS */

//Create a loop with sensor parameter and delay timing
void updateButtonState(int buttonPin, int& buttonState) {
  int currentState = digitalRead(buttonPin);

  if (currentState != buttonState) {
    delay(DEBOUNCE_DELAY);
    currentState = digitalRead(buttonPin);
    if (currentState != buttonState) {
      buttonState = currentState;
      if (buttonState == LOW) {
        buttonState = BUTTON_PRESSED;
      } else {
        buttonState = BUTTON_RELEASED;
      }
    }
  }
}

void displaySensor(int sensor) {
  switch (sensor) {
    case SENSOR_LIGHT:
      Light();
      BH1750toSDI12();
      break;
    case SENSOR_PRESSURE:
      AQ();
      BME680toSDI12();
      break;
    default:
      break;
  }
}

void displayBothSensors() {
  if (millis() - lastSensorUpdateTime >= sensorUpdateInterval) {
    Light();
    BH1750toSDI12();
    AQ();
    BME680toSDI12();
    delay(3000);
    lastSensorUpdateTime = millis();
  }
}

//Saving sensor data menu
void SaveSensorData(){
    // Read sensor data from BH1750 and BME680
    String sensordata = "";
    float lightData = lightMeter.readLightLevel();
    String aqData = getAQData();
    String time = getTime();
    // Store sensor data in sensordata variable
    sensordata = "Light: " + String(lightData) + " lx\n" + "Air Quality: " + aqData + "\n" + time;
    // Save sensor data to SD card
    WriteSD(file, sensordata);
    //Print status on LCD
    Serial.println("Data saved into sensor_data.txt");
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(5, 50);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.println("Data saved into sensor_data.txt");
    delay(2000);
}

// Define string that contain RTC timing data
String getTime() {
DateTime now = rtc.now();
  String time = "Date: " + String(now.year()) + "/" + String(now.month()) + "/" + String(now.day()) + "\n" +
                "Time: " + String(now.hour()) + ":" + String(now.minute()) +  ":" + String(now.second());

  return time;
}

// Define string that contain BME680 sensor's data
String getAQData() {
bme.performReading();
  String data = "Temperature = " + String(bme.temperature) + " *C\n" +
                "Pressure = " + String(bme.pressure / 100.0) + " hPa\n" +
                "Humidity = " + String(bme.humidity) + " %\n" +
                "Gas = " + String(bme.gas_resistance / 1000.0) + " KOhms\n" +
                "Approx. Altitude = " + String(bme.readAltitude(SEALEVELPRESSURE_HPA)) + " m";

  return data;
}

//Interruption state
void handleInterrupt() {
  isInterrupted = true;
}

//Print BME680 data to SDI12 
void BME680toSDI12() {
      bme.performReading();
      SDI12Send(String(bme.temperature) + String(" *C"));
      SDI12Send(String(bme.pressure / 100.0) + String(" HPa"));
      SDI12Send(String(bme.humidity) + String(" %"));
      SDI12Send(String(bme.gas_resistance / 1000.0) + String(" KOhms"));
      SDI12Send(String(bme.readAltitude(SEALEVELPRESSURE_HPA)) + String(" m"));
}


//Print BH1750 data to SDI12 
void BH1750toSDI12() {
      uint16_t lux = lightMeter.readLightLevel();
      SDI12Send(String(lux) + String(" lx"));
}

//Displaying RTC module's time
void Time() {
    DateTime now = rtc.now();
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
}

// Display BME680 data to LCD
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

// Display BH1750 data to LCD
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

//Resetting window, waiting for new inputs
void resetFunc() {
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextSize(1);

  tft.setCursor(50, 60);
  tft.setTextColor(ST77XX_RED);
  tft.println("Reseting...");
}

//Resetting window, waiting for new inputs
void Interruption() {
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextSize(1);

  tft.setCursor(35, 60);
  tft.setTextColor(ST77XX_BLUE);
  tft.println("Interruption...");
  delay(1000);
}
