#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 6;
const int LOADCELL_SCK_PIN = 5;
const int flameSensorPin = 3;
const int mq2GasSensorPin = A2;


HX711 scale;

// Thresholds for gas level (weight), flame sensor, and MQ2 gas sensor
const int weightThreshold = 200;  // Adjust as needed
const int flameThreshold = 50;       // Adjust as needed
const int mq2GasThreshold = 100;     // Adjust as needed

SoftwareSerial gsmSerial(7, 8); // RX, TX
String phoneNumber = ""; // Replace with your actual phone number

// LCD display configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
 Serial.begin(9600);
  Serial.println("HX711 load cell");
   pinMode(flameSensorPin, INPUT);

  // GSM module setup
  gsmSerial.begin(9600);
  
  // LCD display setup
  lcd.begin(16, 2);
  //lcd.print("Gas, Flame, and MQ2 Monitoring");

  Serial.println("Initializing the scale");

  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());			// print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  	// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided
						// by the SCALE parameter (not set yet)

  scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();				        // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
						// by the SCALE parameter set with set_scale

  Serial.println("Readings:");
}

void loop() {

  int gasLevel = analogRead(mq2GasSensorPin);
  Serial.print("Gas Level: ");
  Serial.println(gasLevel);
   if (gasLevel > mq2GasThreshold) {
    Serial.println("MQ2 Gas detected. Alert!");
      sendSMS("MQ2 Gas detected. Alert!");
    }


int flameValue = digitalRead(flameSensorPin);
    Serial.print("Flame Sensor Value: ");
    Serial.println(flameValue);
if (flameValue == 0 ) {
  Serial.println("Flame detected. Alert!");
      sendSMS("Flame detected. Alert!");
    }

  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 1);
  Serial.println("Gms");
   if (int(scale.get_units(10), 1) < weightThreshold) {
    Serial.println("Gas level is low");
      Serial.println("Booking is initiated");
    }

  scale.power_down();			        // put the ADC in sleep mode
  delay(5000);
  scale.power_up();

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Gas: " + String(gasLevel));
  lcd.setCursor(2, 1);
  lcd.print("Weight: " + String(scale.get_units(10), 1) + " g");

  delay(1000); 
}


void sendSMS(String message) {
  gsmSerial.println("AT+CMGF=1"); // Set SMS mode to text
  delay(100);
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");
  delay(100);
  gsmSerial.println(message);
  delay(100);
  gsmSerial.write(26); // Send Ctrl+Z to finish message
  delay(1000); // Wait for the message to be sent
}