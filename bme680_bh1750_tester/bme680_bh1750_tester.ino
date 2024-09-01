#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <BH1750.h>
#include <SDI12.h>
#include <DueTimer.h>

//BME680 Setup
Adafruit_BME680 bme;

//BH1750 Setup
BH1750 lightMeter(0x23);

//SDI-12 Setup
#define DIRO 7
#define SDI12_UART Serial1

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

// Define board-specific timer and pins for SDI-12
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
  #define SDI12_TIMER1
  #define SDI12_PIN_2
#elif defined(ARDUINO_AVR_NANO_33_IOT)
  #define SDI12_TIMER4
  #define SDI12_PIN_10
#else
  
#endif

String command;
int deviceAddress = 0; //default address for sensor 1
String deviceIdentification = "allccccccccmmmmmmvvvxxx";

void setup() {
  //Arduino IDE Serial Monitor
  Serial.begin(9600);

  // ================ BME680 ================
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
    // Set the temperature, pressure and humidity oversampling
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setPressureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);

  // ================ BH1750 ================
  Wire.begin();
  lightMeter.begin();

  // ================ SDI-12 ================
  SDI12_UART.begin(1200, SERIAL_7E1);  //SDI-12 UART, configures serial port for 7 data bits, even parity, and 1 stop bit
  pinMode(DIRO, OUTPUT);               //DIRO Pin

  // Set SDI-12 receive mode
  #if defined(SDI12_TIMER1)
    SDI12.begin(&SDI12_UART, SDI12_PIN_2, DIRO, TIMER1_COMPA_vect, true); // Setup ISR for timer1 COMPA vector
  #elif defined(SDI12_TIMER4)
    SDI12.begin(&SDI12_UART, SDI12_PIN_10, DIRO, TIMER4_COMPA_vect, true); // Setup ISR for timer4 COMPA vector
  #endif

  // Set SDI-12 address
  SDI12.setAddress(deviceAddress);

  //HIGH to Receive from SDI-12
  digitalWrite(DIRO, HIGH);
}

void loop() {
int byte;
//Receive SDI-12 over UART and then print to Serial Monitor
if(Serial1.available()) {
byte = Serial1.read(); //Reads incoming communication in bytes
//Serial.println(byte);
if (byte == 33) { //If byte is command terminator (!)
SDI12Receive(command);
command = ""; //reset command string
} else {
if (byte != 0) { //do not add start bit (0)
command += char(byte); //append byte to command string
}
}
}

// Read BME680 sensor data and print to Serial Monitor
Serial.print("Temperature = ");
Serial.print(bme.readTemperature());
Serial.println(" *C");

Serial.print("Pressure = ");
Serial.print(bme.readPressure() / 100.0F);
Serial.println(" hPa");

Serial.print("Humidity = ");
Serial.print(bme.readHumidity());
Serial.println(" %");

// Read BH1750 sensor data and print to Serial Monitor
uint16_t lightLevel = lightMeter.readLightLevel();
Serial.print("Light level = ");
Serial.print(lightLevel);
Serial.println(" lux");

// Wait for 1 second
delay(1000);
}