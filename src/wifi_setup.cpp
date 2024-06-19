#include "wifi_setup.h"

const char *ssid = "MSI5554";
const char *pw = "12345678";

void connectWiFi(){
  WiFi.mode(WIFI_OFF);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  Serial.println("Connecting to Wifi");

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println("Connecting to wifi");
  }

  Serial.println("Connected to ");
  Serial.println(ssid);
}
