/*********************************************************************
 * DF Pong Controller
 * 
 * This program implements a Bluetooth Low Energy controller for Pong.
 * It sends movement data to a central device running in the browser and
 * provides audio feedback through a buzzer.
 *
 * Game Link : https://digitalfuturesocadu.github.io/df-pong/
 * 
 * Movement Values:
 * 0 = No movement / Neutral position
 * 1 = UP movement (paddle moves up)
 * 2 = DOWN movement (paddle moves down)
 * 3 = Handshake signal (used for initial connection verification)
 * 
 * Key Functions:
 * - handleInput(): Process the inputs to generate the states
 * - sendMovement(): Sends movement data over BLE (0-3)
 * - updateBLE(): Handles BLE connection management and updates
 * - updateBuzzer(): Provides different buzzer patterns for different movements
 * 
 * Key Variables:
 * - currentMovement: Stores current movement state (0-2)
 * - deviceName : GIVE YOUR DEVICE AN APPROPRIATE NAME
 * - LED_PIN : It is important to see the status of the arduino through the LED. 
      if you can see the built-in add an external one and update the pin it is connected to
 * 

 *********************************************************************/



#include <ArduinoBLE.h>
#include "ble_functions.h"
#include "buzzer_functions.h"
//Since code is split over multiple files, we have to include them here


// ============================================
// IMPORTANT: SET YOUR DEVICE NUMBER HERE (1-25)
// ============================================
const int DEVICE_NUMBER = 20;  // ← CHANGE THIS TO YOUR ASSIGNED NUMBER!
// ============================================

// Device name is generated from device number
String deviceNameStr = "DFPONG-" + String(DEVICE_NUMBER);
const char* deviceName = deviceNameStr.c_str();

// Pin definitions buzzer/LED
const int BUZZER_PIN = 11;       // Pin for haptic feedback buzzer
const int LED_PIN = LED_BUILTIN; // Status LED pin

// Movement state tracking
int currentMovement = 0;         // Current movement value (0=none, 1=up, 2=down, 3=handshake)




void setup() 
{
  
  Serial.begin(9600);
  // DO NOT use while(!Serial) - it blocks startup without Serial Monitor
  // Small delay allows Serial Monitor to catch output if connected
  delay(1000);
  
  Serial.println("=== DF Pong Controller Starting ===");
  
  // Configure LED for connection status indication
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize Bluetooth Low Energy with device name, number, and status LED
  setupBLE(deviceName, DEVICE_NUMBER, LED_PIN);
  
  // Initialize buzzer for feedback
  setupBuzzer(BUZZER_PIN);
}

void loop() 
{
  // Update BLE connection status and handle incoming data
  updateBLE();
  
  //read the inputs te determine the current state
  //results in changing the value of currentMovement
  handleInput();

  //send the movement state to P5  
  sendMovement(currentMovement);

  //make the correct noise
  updateBuzzer(currentMovement);
  
  
}

void handleInput() 
{
  // This part reads the analog values from the pressure sensors on both pages
  int leftSensorValue = analogRead(A0);   // Left page sensor is connected to A0
  int rightSensorValue = analogRead(A1);  // Right page sensor is connected to A1

  // This is to set the threshold values for each page. It account for the natural pressure when the book is open
  // Left page has less natural pressure from opening the book, so the threshold is lower
  int leftThreshold  = 300;
  // Right page naturally bends downward more and carries extra pressure, so the threshold is higher
  int rightThreshold = 700;

  // This determines whether each page is being actively pressed
  bool leftPressed  = leftSensorValue  > leftThreshold;
  bool rightPressed = rightSensorValue > rightThreshold;

  // --- Input logic to decide paddle movement ---

  // Instance 1: Neither page is pressed above its threshold - paddle stays still
  if (!leftPressed && !rightPressed) {
    currentMovement = 0;
  }

  // Instance 2: Only the left page is pressed - move paddle LEFT/UP
  else if (leftPressed && !rightPressed) {
    currentMovement = 1;
  }

  // Instnce 3: Only the right page is pressed - move paddle RIGHT/DOWN
  else if (!leftPressed && rightPressed) {
    currentMovement = 2;
  }

  // Instance 4: Both pages are pressed → move in the direction of the stronger pressure
  else {
    if (leftSensorValue > rightSensorValue) {
      currentMovement = 1; // Left page pressure is stronger → move LEFT/UP
    } else {
      currentMovement = 2; // Right page pressure is stronger → move RIGHT/DOWN
    }
  }

  // Debugging output to monitor sensor readings and current movement
  Serial.print("Left: "); Serial.print(leftSensorValue);
  Serial.print(" | Right: "); Serial.print(rightSensorValue);
  Serial.print(" | Movement: "); Serial.println(currentMovement);
}
