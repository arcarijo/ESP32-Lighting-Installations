/**
   ESPNOW - Basic communication - Master
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Master module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Master >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave

   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Global copy of slave
esp_now_peer_info_t slave;
#define CHANNEL 0
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0

uint8_t data = 0;
// send data

int mac[6] = { 0x24, 0xDC, 0xC3, 0x98, 0x80, 0xC9 };
int piezoPin[4] = { 34, 35, 33, 32 };
unsigned long startTime[4] = { 0, 0, 0, 0 };
int piezo = 0;

// Init ESP Now with fallback
void initESPNow() {
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

// Set MAC address of slave
void setSlave() {
  //int8_t scanResults = WiFi.scanNetworks();
  memset(&slave, 0, sizeof(slave));

  for (int i = 0; i < 6; ++i) {
    slave.peer_addr[i] = (uint8_t)mac[i];
  }

  slave.channel = CHANNEL;  // pick a channel
  slave.encrypt = 0;        // no encryption
}

// Check if the slave is already paired with the master.
// If not, pair the slave with master
bool pairSlave() {

  //failsave in case the channel got messed up somehow
  if (slave.channel != CHANNEL) {
    Serial.println("No Slave found to process");
    return false;
  }

  Serial.print("Slave Status: ");

  // check if the peer exists
  if (esp_now_is_peer_exist(slave.peer_addr)) {
    Serial.println("Already Paired");
    return true;
  }
  // Slave not paired, attempt pair
  else {
    esp_err_t addStatus = esp_now_add_peer(&slave);

    if (addStatus == ESP_OK) {
      // Pair success
      Serial.println("Pair success");
      return true;
    } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW Not Init");
      return false;
    } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
      return false;
    } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
      Serial.println("Peer list full");
      return false;
    } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
      Serial.println("Out of memory");
      return false;
    } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
      Serial.println("Peer Exists");
      return true;
    } else {
      Serial.println("Not sure what happened");
      return false;
    }
  }
}

// send data
void sendData() {
  //make sure there actually is a slave paired before sending data
  //if no slave is found, pair the slave here
  pairSlave();

  //send data
  const uint8_t *peer_addr = slave.peer_addr;
  esp_err_t result = esp_now_send(peer_addr, (uint8_t *)&data, sizeof(data));

  Serial.print("Send Status: ");
  if (result == ESP_OK) {
    Serial.println("Success");
  } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW not Init.");
  } else if (result == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
    Serial.println("Internal Error");
  } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
}

// callback when data is sent from Master to Slave
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: ");
  Serial.println(macStr);
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  pinMode(0, INPUT);
  WiFi.mode(WIFI_STA);  //station mode
  initESPNow();
  // Sets OnDataSent() as callback function for debugging
  esp_now_register_send_cb(onDataSent);
  setSlave();
}

void loop() {

  for (int i = 0; i <= 4; i++) {
    //on click
    if (analogRead(piezoPin[i]) > 1000) {
      Serial.println(piezoPin[i]);
      bitWrite(data, i, 1);
      startTime[i] = millis();
      sendData();
    }

    //duration ends
    if (millis() - startTime[i] >= 500 && bitRead(data, i) != 0) {
      bitWrite(data, i, 0);
      sendData();
    }
  }
  Serial.println("------------------------------------------------------------");
}
