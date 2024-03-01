#ifndef WIFI_HELPER
#define WIFI_HELPER
#include <Arduino.h>
#include <WiFi.h>
#include "config_file.h"
#include <stdbool.h>

IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 0, 1);
IPAddress local_IP(192, 168, 0, 7);

void disconnect_wifi(){
  WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
}

void setup_wifi(bool staticaddr){
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    if(staticaddr){
      if (WiFi.config(local_IP, gateway, subnet))
        Serial.println("Static IP Configured");
      else Serial.println("Static IP Configuration Failed");
    }
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}


#endif