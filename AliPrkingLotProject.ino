#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // Adjust the address and columns according to your LCD specifications
SoftwareSerial mySerial(3, 2); // SIM808 Tx & Rx = Arduino #3 & #2
DS3231 rtc;

// Define the input pin for the sensors
int sensorPins[] = {5, 6, 7, 8, 9, 10, 11, 12};
int numSensors = 8;
int lastSensorStates[8] = {1}; // Initialize to 1 for inverse behavior
bool parkingOpen = false;
bool parkingFullMessageSent = false; // Flag to track if parking full message has been sent
const int buttonPin = 13; // Assuming you connect the push button to pin 13
unsigned long startTime[8] = {0}; // Array to store the start time for each slot
float cost[8] = {0}; // Array to store the cost for each slot
const unsigned long costPerMinute = 10; // Cost per minute in dollars
void setup() {
  lcd.begin();
  lcd.clear();
  pinMode(13, INPUT_PULLUP); // Assuming the button is connected to pin 13 with internal pull-up resistor
 
  for (int i = 0; i < numSensors; i++) {
    pinMode(sensorPins[i], INPUT);
    lastSensorStates[i] = digitalRead(sensorPins[i]);
    attachInterrupt(digitalPinToInterrupt(sensorPins[i]), sensorInterrupt, CHANGE);
  }
  Serial.begin(9600);
  Serial.println("Initializing...");
  mySerial.begin(9600);
  delay(1000);
  rtc.setYear(2024);
  rtc.setMonth(1);
  rtc.setDate(25);
  rtc.setDoW(SUNDAY);
  rtc.setHour(12);
  rtc.setMinute(0);
  rtc.setSecond(0);
}
void loop() {
  static unsigned long welcomeTime = millis();
  static boolean welcomeDisplayed = false;
  static boolean buttonPressed = false;

  // Reset cost array to zero at the beginning of each loop iteration
  memset(cost, 0, sizeof(cost));

  if (!welcomeDisplayed) {
    lcd.setCursor(0, 1);
    lcd.print("Welcome to Ali's");

    lcd.println();
    lcd.setCursor(0, 2);
    lcd.print("Parking ");
    if (millis() - welcomeTime > 3000) {
      lcd.clear();
      welcomeDisplayed = true;
    }
    delay(1000);
    return;
  }

  int freeSlots = 0; // Count of free slots

  for (int i = 0; i < numSensors; i++) {
    int sensorValue = digitalRead(sensorPins[i]);
    updateLCD(i, sensorValue);
    if (sensorValue != lastSensorStates[i]) {
      // ... (rest of your code remains unchanged)
    }
    freeSlots += (sensorValue == HIGH); // Increment freeSlots for each LOW (free) sensor
  }
 
  lcd.setCursor(4, 0);
  lcd.print("Have slots:");
  lcd.print(freeSlots); // Display the number of available (free) slots

  // Check if the button is pressed and the buttonPressed flag is false
  if (digitalRead(buttonPin) == HIGH && !buttonPressed) {
    lcd.clear();
    unsigned long startTime = millis();
    buttonPressed = true; // Set the flag to true to indicate the button has been pressed
    //Serial.println(millis() - startTime);
    // Display the information for each slot on the LCD for 10 seconds
    while (millis() - startTime < 10000) {
      for (int i = 0; i < numSensors; i++) {
        int row;
        int col;
        if (i < 4) {
          row = i;
          col = 0;
        } else {
          row = i - 4;
          col = 10;
        }
        lcd.setCursor(col, row);
        lcd.print("S" + String(i + 1) + ":" );
        lcd.print("$");
        lcd.print(cost[i]/*, 2*/);
      }
      delay(1000);
    }

    // Clear the LCD after 10 seconds
    lcd.clear();
  }

  // Reset the buttonPressed flag when the button is not pressed
  if (digitalRead(buttonPin) == LOW) {
    buttonPressed = false;
  }
  delay(1000); // Adjust the delay based on your needs
}
void sensorInterrupt() {
  // This function will be called when any IR sensor state changes
}
void updateLCD(int sensorIndex, int sensorValue) {
  int row;
  int col;
  if (sensorIndex < 4) {
    row = 1;
col = sensorIndex * 5; // Each slot takes 5 columns
  } else {
    row = 2;
    col = (sensorIndex - 4) * 5; // Each slot takes 5 columns
  }
  lcd.setCursor(col, row);
  lcd.print("S" + String(sensorIndex + 1) + ":" + (sensorValue == HIGH ? "F" : "O")); // Use "F" for free and "O" for occupied
  // lcd.print(" $");
  // lcd.print(cost[sensorIndex], 2); // Print the cost with two decimal places
}
void cost_per_hour(int milisanie) {}
void sendSMS(String phoneNumber, String message) {
  mySerial.println("AT+CMGF=1");
  delay(1000);
  mySerial.print("AT+CMGS=\"");
  mySerial.print(phoneNumber);
  mySerial.println("\"");
  delay(1000);
  mySerial.print(message);
  delay(100);
  mySerial.write(26);
  delay(1000);
}
bool areAllSlotsOccupied() {
  for (int i = 0; i < numSensors; i++) {
    if (digitalRead(sensorPins[i]) == HIGH) {
      return false; // At least one slot is free, not all occupied
    }
  }
  return true; // All slots are occupied
}