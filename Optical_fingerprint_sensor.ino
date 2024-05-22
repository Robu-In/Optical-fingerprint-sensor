#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h> // Include the Servo library

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Use hardware I2C for OLED

SoftwareSerial mySerial(2, 3);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Servo servo; // Define servo object

const char accessGranted[] PROGMEM = "Access Granted";
const char accessDenied[] PROGMEM = "Access Denied";

void setup() {
  Serial.begin(9600);
  while (!Serial);  // Wait for serial port to connect

  servo.attach(9); // Attach servo to pin 9

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.clearDisplay(); // Clear the buffer
  display.setTextSize(1); // Set text size
  display.setTextColor(SSD1306_WHITE); // Set text color
  display.setCursor(0,0); // Set cursor position
  display.println(F("fingertest")); // Print title
  display.display(); // Show initial title

  pinMode(4, OUTPUT); // LED pin for found fingerprint
  pinMode(5, OUTPUT); // LED pin for fingerprint mismatch

  finger.begin(57600);

  if (!finger.verifyPassword()) {
    display.clearDisplay(); // Clear the buffer
    display.setCursor(0,0); // Set cursor position
    display.println(F("Did not find fingerprint sensor :(")); // Print message
    display.display(); // Show message
    while (true);
  }

  display.clearDisplay(); // Clear the buffer
  display.setCursor(0,0); // Set cursor position
  display.println(F("Found fingerprint sensor!")); // Print message
  display.display(); // Show message

  finger.getTemplateCount();
  Serial.print(F("Sensor contains ")); Serial.print(finger.templateCount); Serial.println(F(" templates"));
  Serial.println(F("Waiting for valid finger..."));
}

void loop() {
  int result = getFingerprintIDez();
  digitalWrite(4, LOW);

  display.clearDisplay(); // Clear the buffer
  display.setCursor(0,0); // Set cursor position
  if (result != -1) {
    if (result == 1) {
      display.println(F("Access Granted")); // Print message
      servo.write(90); // Rotate servo to 90 degrees
      delay(3000); // Keep servo at 90 degrees for 3 seconds
      servo.write(0); // Return servo to 0 degrees
    } else {
      display.println(F("Access Denied")); // Print message
    }
    display.display(); // Show message
    delay(3000); // Display message for 3 seconds
  }
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    return -1;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    return -1;
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    digitalWrite(5, HIGH); // Turn on LED for fingerprint mismatch
    delay(3000); // Wait for 3 seconds
    digitalWrite(5, LOW); // Turn off LED for fingerprint mismatch
    return 0;
  }

  digitalWrite(4, HIGH); // Turn on LED for found fingerprint
  delay(3000); // Wait for 3 seconds
  digitalWrite(4, LOW); // Turn off LED for found fingerprint
  Serial.print(F("Found ID #")); Serial.print(finger.fingerID);
  Serial.print(F(" with confidence of ")); Serial.println(finger.confidence);

  return 1;
}
