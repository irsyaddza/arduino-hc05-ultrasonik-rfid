#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <SoftwareSerial.h>

#define TRIG_PIN 7
#define ECHO_PIN 6
#define BT_RX 11
#define BT_TX 10

PN532_I2C pn532_i2c(Wire);
PN532 nfc(pn532_i2c);
LiquidCrystal_I2C lcd(0x27, 16, 2); 
SoftwareSerial bluetooth(BT_RX, BT_TX);


bool rfidActive = false;
bool ultrasonicActive = false;

void setup() {

  Serial.begin(9600);
  bluetooth.begin(9600);
  

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Sistem Aktif");
  

  nfc.begin();
  
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    lcd.clear();
    lcd.print("PN532 tidak ada");
    while (1); // Halt
  }
  

  nfc.SAMConfig();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  delay(1000);
}

void loop() {
  checkBluetoothCommands();
  
  if (rfidActive) {
    readRFID();
  }
  
  if (ultrasonicActive) {
    readUltrasonic();
  }
}

void checkBluetoothCommands() {
  if (bluetooth.available()) {
    String command = bluetooth.readStringUntil('\n');
    command.trim();
    
    if (command == "LED 1 ON") {
      rfidActive = true;
      lcd.clear();
      lcd.print("RFID Active");
    }
    else if (command == "LED 1 OFF") {
      rfidActive = false;
      lcd.clear();
      lcd.print("RFID Inactive");
    }
    else if (command == "LED 2 ON") {
      ultrasonicActive = true;
      lcd.clear();
      lcd.print("Ultrasonic Active");
    }
    else if (command == "LED 2 OFF") {
      ultrasonicActive = false;
      lcd.clear();
      lcd.print("Ultrasonic Off");
    }
    else if (command == "LED 3 ON/OFF" || command == "5") {
      lcd.clear();
      lcd.print("Display Reset");
      delay(1000);
      lcd.clear();
    }
  }
}

void readRFID() {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    String cardID = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) {
        cardID += "0";
      }
      cardID += String(uid[i], HEX);
    }
    
    lcd.clear();
    lcd.print("Card ID:");
    lcd.setCursor(0, 1);
    lcd.print(cardID);
    
    delay(1000);
  }
}

void readUltrasonic() {
  // Clear trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Send trigger pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read echo pulse
  long duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculate distance
  int distance = duration * 0.034 / 2;
  
  // Display on LCD
  lcd.clear();
  lcd.print("Jarak: ");
  lcd.print(distance);
  lcd.print(" CM");
  
  delay(500);
}