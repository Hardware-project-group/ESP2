#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJSON.h>

#include "Finger_check.h"
#include "enroll.h"
#include "wifi_setup.h"
#include "Display.h"

HardwareSerial serialPort(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);

HTTPClient http;
WebServer server(80);

uint8_t id;
uint8_t fingerId;
bool passcodeVerified = false;
String postdata = "tag=";
int ItemsScanned = 0;
const int buttonPin = 26;
int buttonState = 0;
int doorState = 0;
String postdataIP;
const int doorsensorPin = 32;

#define SS_PIN 5
#define RST_PIN 14
MFRC522 mfrc522(SS_PIN, RST_PIN);
bool borrowed_item1 = false;
bool borrowed_item2 = false;
String content;


void handleFingerprint() {
  Serial.println("Handling fingerprint request...");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  http.begin("http://192.168.137.1:5000/update-inside-finger");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  if (server.hasArg("fingerID")) {
    lcd.clear();
    lcd.print("Start Enrolling Fingerprint");
    String paramValueStr = server.arg("fingerID");
    int paramValue = paramValueStr.toInt();
    Serial.print("Received parameter value: ");
    Serial.println(paramValue);

    int enrollStatus = -1;
    while (enrollStatus != 1) {
      lcd.clear();
      lcd.print("Enrolling fingerprint...");
      enrollStatus = getFingerprintEnroll(paramValue);
      if (enrollStatus != 1) {
        lcd.print("Something went wrong..Try again!");
      }
      delay(2000);
    }
    server.send(200, "text/plain", "Fingerprint enroll successfully!");
    String value = String(paramValue);
    Serial.print(value);
    postdata = "id=" + value;
    Serial.print("POST data: ");
    Serial.print(postdata);
    int httpResponseCode = http.POST(postdata);
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(payload);
    } else {
      Serial.print("Error on sending POST request: ");
      Serial.println(httpResponseCode);
    }
    buttonState = 0;
    http.end();
    lcd.clear();
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Scan items:");
    lcd.setCursor(0, 1);
    lcd.print("Scanned items: " + String(ItemsScanned));

  } else {
    server.send(400, "text/plain", "Something went wrong, contact service");
  }

}

void handleRoot() {
  server.send(200, "text/plain", "Welcome to the ESP32 Fingerprint Enrollment System");
}

void tagHandler(String tagId){
    http.begin("http://192.168.137.1:5000/process-tag");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    postdata = postdata + tagId;
    Serial.println(tagId);
    Serial.println(postdata);
    int httpcode = http.POST(postdata);
    String payload = http.getString();
    Serial.print(payload);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    String message = doc["message"];
    Serial.println(message);
    if(message == "Tag processed successfully."){
      ItemsScanned++;
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Scan items:");
      lcd.setCursor(0, 1);
      lcd.print("Scanned items: " + String(ItemsScanned));

    }else if(message == "Tag ID added to tagdetails."){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("New item added ");
      lcd.setCursor(0, 1);
      lcd.print("to Stock!");
      delay(200);
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Scan items:");
      lcd.setCursor(0, 1);
      lcd.print("Scanned items: " + String(ItemsScanned));

    }else{
      lcd.clear();
      lcd.print(message);
      delay(1000);
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("Scan items:");
      lcd.setCursor(0, 1);
      lcd.print("Scanned items: " + String(ItemsScanned));
    }
    http.end();
    postdata = "tag=";
}

void handleRFID() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }else{
    content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    content.toUpperCase();
    Serial.println(content);
    digitalWrite(13, HIGH);
    digitalWrite(12, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    digitalWrite(12, LOW);

    tagHandler(content);
  }
}

void sendIp(String ip){
    http.begin("http://192.168.137.1:5000/SendIp");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    postdataIP = "Board=ESP2&ip=" + ip;
    int httpResponseCode = http.POST(postdataIP);
    Serial.println("IpSend Statues: ");
    Serial.println(httpResponseCode);
    http.end();
}

void setup() {
  Serial.begin(9600);
  Displaysetup();
  connectWiFi();
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(doorsensorPin, INPUT_PULLUP);

  while (!Serial) {
    ; // Wait for the serial port to connect
  }

  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // Initialize the fingerprint sensor
  serialPort.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }
  delay(1000);

  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();

  // Setup routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/enroll", HTTP_POST, handleFingerprint);
  server.begin();
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Scan items:");
  lcd.setCursor(0, 1);
  lcd.print("Scanned items: " + String(ItemsScanned));
  IPAddress ip = WiFi.localIP();
  String ipString = ip.toString();
  sendIp(ipString);
  delay(1000);
  getIpOfEsp1();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  server.handleClient();
  handleRFID();
  doorState  = digitalRead(doorsensorPin);
  //Serial.println(doorState);
  buttonState = digitalRead(buttonPin);
  // Serial.println(buttonState);
  if(buttonState == HIGH){
    while (getFingerprintID() == 2)
      ;
    delay(1000);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Scan items:");
    lcd.setCursor(0, 1);
    lcd.print("Scanned items: " + String(ItemsScanned));
  }
  
}
