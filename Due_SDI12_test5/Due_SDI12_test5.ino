#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <BH1750.h>

//BME680 Setup
Adafruit_BME680 bme;

//BH1750 Setup
BH1750 lightMeter(0x23);

//SDI-12 Setup
#define DIRO 7

String command;
int deviceAddress = 0;
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
  Serial1.begin(1200, SERIAL_7E1);  //SDI-12 UART, configures serial port for 7 data bits, even parity, and 1 stop bit
  pinMode(DIRO, OUTPUT);               //DIRO Pin

  //HIGH to Receive from SDI-12
  digitalWrite(DIRO, HIGH);
}

void loop() {
  int byte;
  //Receive SDI-12 over UART and then print to Serial Monitor
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
}


void SDI12Receive(String input) {
  //convert device address to string
  String address = String(deviceAddress);

      if ((String(input.charAt(1)) == "?") ){
      uint16_t lux = lightMeter.readLightLevel();
      SDI12Send(String(lux));

      bme.performReading();
      SDI12Send(String(bme.temperature));
      Serial.println("Responding to TEST command");
    }

  //Determines if the command is addressed for this device
  if (String(input.charAt(0)) == address) {  
    //Repond to Start Measurement command "aTEST!"   **Notice: Not correctly implemented, this only demostrates command and usuage of I2C sensors
    if ((String(input.charAt(1)) == "T") && (String(input.charAt(2)) == "E")&& (String(input.charAt(3)) == "S")&& (String(input.charAt(4)) == "T") ) {
      uint16_t lux = lightMeter.readLightLevel();
      SDI12Send(String(lux));

      bme.performReading();
      SDI12Send(String(bme.temperature));
      Serial.println("Responding to TEST command");
    }
    
  }  
}

void SDI12Send(String message) {
  Serial.print("message: "); Serial.println(message);
  
  digitalWrite(DIRO, LOW);
  delay(100);
  Serial1.print(message + String("\r\n"));
  Serial1.flush();    //wait for print to finish
  Serial1.end();
  Serial1.begin(1200, SERIAL_7E1);
  digitalWrite(DIRO, HIGH);
  //secondruntoken = 0;
}