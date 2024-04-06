/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <esp_now.h>
#include <WiFi.h>

uint8_t data = 0;
int piezo[4] = { 16, 17, 18, 19 };
const int LED = 13;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  //memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(*data);

  
  for (int i = 0; i < 4; i++) {
    if (bitRead(*data, i)) {
      digitalWrite(LED, LOW);
      digitalWrite(piezo[i], LOW);
    } else {
      digitalWrite(LED, HIGH);
      digitalWrite(piezo[i], HIGH);
    }
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(LED, OUTPUT);

  for (int i = 0; i < 4; i++) {
    pinMode(piezo[i], OUTPUT);
  }
}

void loop() {
}
