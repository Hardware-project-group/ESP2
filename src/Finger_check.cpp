#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <HTTPClient.h>
#include <ArduinoJSON.h>
#include "Finger_check.h"
#include "wifi_setup.h"
#include "Display.h"

String serverAddress; // Update with your server address
String postData;
String payload;
int httpCode;


void getIpOfEsp1(){
    HTTPClient http;
    http.begin("http://10.13.127.54/TestEsp/GetIp.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String postData= "id=ESP1";
    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
            String response = http.getString();  // Get the response to the request
            Serial.println(httpResponseCode);    // Print the response code
            Serial.println(response);            // Print the response payload

            // Parse the JSON response
            JsonDocument doc;
            deserializeJson(doc, response);

            String ip_address = doc["ip_address"];
            if (ip_address) {
                Serial.print("IP Address: ");
                Serial.println(ip_address);
                serverAddress = "http://" + String(ip_address) + "/doorOpen";
                Serial.println(serverAddress);
            } else {
                Serial.println("Failed to retrieve IP address.");
            }

        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        // Free resources
        http.end();

}


void sendData(int id){
    HTTPClient http;
    String serverAddress2 = "http://10.13.127.54/TestEsp/hangleExit.php";
    http.begin(serverAddress2);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String postdata = "id=" + String(id);
    Serial.println(postdata);
    int httpcode = http.POST(postdata);
    String payload = http.getString();
    Serial.print(payload);
    http.end();
    delay(100);
    
    http.begin(serverAddress);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
    postData = "id=" + String(finger.fingerID);
    Serial.println(postData);
    httpCode = http.POST(postData);
    payload = http.getString();
    Serial.print("httpCode: ");
    Serial.println(httpCode); // Print HTTP return code
    Serial.print("payload: ");
    Serial.println(payload);  // Print request response payload
    http.end();
    
}


uint8_t getFingerprintID() {

    Displaysetup();
    uint8_t p = finger.getImage();
    switch (p) {
        case FINGERPRINT_OK:
            lcd.clear();
            Serial.println("Image taken");
            lcd.print("Remove Finger");
            delay(1000);
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println("No finger detected");
            lcd.clear();
            lcd.print("Place Finger..");
            delay(1000);
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            lcd.clear();
            lcd.print("Try again!");
            delay(1000);
            return p;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            lcd.clear();
            lcd.print("Try again!");
            delay(1000);
            return p;
        default:
            Serial.println("Unknown error");
            lcd.clear();
            lcd.print("Try again!");
            delay(1000);
            return p;
    }

    p = finger.image2Tz();
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image converted");
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");
            lcd.clear();
            lcd.print("Image too messy.Try again!");
            delay(1000);
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            lcd.clear();
            lcd.print("Try again!");
            delay(1000);
            return p;
        case FINGERPRINT_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            lcd.clear();
            lcd.print("Something went wrong!");
            delay(1000);
            return p;
        case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            lcd.clear();
            lcd.print("Something went wrong!");
            delay(1000);
            return p;
        default:
            Serial.println("Unknown error");
            delay(1000);
            return p;
    }

    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK) {
        Serial.println("Fingerprint Confirmed");
        delay(100);
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        delay(1000);
        return p;
    } else if (p == FINGERPRINT_NOTFOUND) {
        Serial.println("Did not find a match");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("You are not to ");
        lcd.setCursor(0, 1);
        lcd.print("Access Warehouse");
        delay(5000);
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    lcd.clear();
    Serial.print("Found ID #");
    lcd.print("Found ID #");
    Serial.print(finger.fingerID);
    lcd.print(finger.fingerID);
    lcd.clear();
    lcd.print("Door opening");
    sendData(finger.fingerID);
    delay(2000);
    Serial.println(" with confidence of ");
    Serial.println(finger.confidence);
    return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK) return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK) return -1;

    // found a match!
    lcd.clear();
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    lcd.print("Found ID #");
    lcd.print(finger.fingerID);
    lcd.clear();
    lcd.print("Door Opening");
    sendData(finger.fingerID);
    delay(2000);
    Serial.println(" with confidence of ");
    Serial.print(finger.confidence);
    return finger.fingerID;
}
