#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>  // Correct Library for ESP8266
#include <Servo.h>

// WiFi Credentials
const char* ssid = "ParkingSystem";
const char* password = "12345678";

// Web Server on port 80
ESP8266WebServer server(80);

// LCD setup
LiquidCrystal_PCF8574 lcd(0x27);  // For most I2C modules address 0x27

// Servo setup
Servo gateServo;

// Pin Mapping
#define IR_SENSOR1 D6  // Entrance IR sensor
#define IR_SENSOR2 D7  // Exit IR sensor
#define SERVO_PIN  D5  // Servo motor signal
#define LCD_SDA    D4  // I2C SDA
#define LCD_SCL    D3  // I2C SCL

int parkingSpaces = 2;  // Total parking spaces

void setup() {
  Serial.begin(115200);

  // Start custom I2C with correct pins
  Wire.begin(LCD_SDA, LCD_SCL);

  // LCD Initialization
  lcd.begin(16, 2);  
  lcd.setBacklight(255);

  // Setup WiFi AP
  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP Started");
  Serial.println(WiFi.softAPIP());

  // Webserver Routes
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // Setup Servo
  gateServo.attach(SERVO_PIN);
  gateServo.write(0);  // Start with gate closed

  // Setup IR sensors
  pinMode(IR_SENSOR1, INPUT);
  pinMode(IR_SENSOR2, INPUT);

  updateLCD();
}

void loop() {
  server.handleClient();

  // Car Entering
  if (digitalRead(IR_SENSOR1) == LOW && parkingSpaces > 0) {
    delay(100); // Debounce
    if (digitalRead(IR_SENSOR1) == LOW) {
      parkingSpaces--;
      openGate();
      updateLCD();
    }
    while (digitalRead(IR_SENSOR1) == LOW); // Wait till car leaves sensor
  }

  // Car Exiting
  if (digitalRead(IR_SENSOR2) == LOW && parkingSpaces < 2) {
    delay(100); // Debounce
    if (digitalRead(IR_SENSOR2) == LOW) {
      parkingSpaces++;
      updateLCD();
    }
    while (digitalRead(IR_SENSOR2) == LOW); // Wait till car leaves sensor
  }
}

// Function to operate Servo Gate
void openGate() {
  for (int pos = 0; pos <= 180; pos += 3) {
    gateServo.write(pos);
    delay(10);
  }
  delay(2000); // Gate remains open
  for (int pos = 180; pos >= 0; pos -= 3) {
    gateServo.write(pos);
    delay(10);
  }
}

// Function to update LCD
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Parking");

  lcd.setCursor(0, 1);
  if (parkingSpaces > 0) {
    lcd.print("Spaces: ");
    lcd.print(parkingSpaces);
  } else {
    lcd.print("No Space!");
  }
}

// Web Page Handler
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5'>";
  html += "<title>Parking System</title>";
  html += "<style>body { background:#e0f7fa; font-family:Arial; text-align:center; }";
  html += "h1 { color:#00796b; margin-top:50px; }";
  html += ".status { margin:20px; padding:20px; font-size:24px; background:#4CAF50; color:white; border-radius:10px; display:inline-block;}";
  html += "</style></head><body>";
  html += "<h1>🚗 Smart Parking System 🚗</h1>";

  if (parkingSpaces > 0) {
    html += "<div class='status'>Available Spaces: ";
    html += parkingSpaces;
    html += "</div>";
  } else {
    html += "<div class='status' style='background:red;'>No Space Available!</div>";
  }

  html += "</body></html>";
  
  server.send(200, "text/html", html);
}