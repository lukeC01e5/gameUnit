#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include "RFID.h" // Include the RFID header
#include "RFIDData.h"
#include "object.h"

// Include the file with the WiFi credentials
#include "arduino_secrets.h"

// Define TFT instance
TFT_eSPI tft = TFT_eSPI(); // Create TFT instance

AsyncWebServer server(80); // Create AsyncWebServer object on port 80

// WiFi credentials
const char* ssid = SECRET_SSID;
const char* pass = SECRET_PASS;

// Global PlayerData instance
PlayerData playerData; // Struct to hold parsed RFID data

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting setup...");

  // Initialize the TFT display
  Serial.println("Initializing TFT...");
  tft.init();
  tft.setRotation(1);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.println("TFT OK");
  Serial.println("TFT initialized");

  // Initialize RFID (defined in RFIDData.cpp)
  initializeRFID();

  // Initialize SPIFFS
  Serial.println("Initializing SPIFFS...");
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    tft.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
  tft.println("SPIFFS OK");

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  tft.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  int wifi_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_attempts < 20)
  {
    delay(1000);
    Serial.print(".");
    tft.print(".");
    wifi_attempts++;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("\nWiFi connection failed.");
    tft.println("\nWiFi failed");
    return;
  }
  Serial.println("\nWiFi connected.");
  tft.println("\nWiFi OK");

  // Setup server routes and handlers

  // Serve the index.html file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Handle form submission
  server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("name", true))
    {
        // Retrieve the name parameter
        String trainerName = request->getParam("name", true)->value();

        // Write name to RFID
        if (writePlayerNameToRFID(trainerName.c_str()))
        {
            Serial.println("RFID Write Successful");
            tft.println("RFID Write Success");
            request->send(200, "text/plain", "Name written to RFID successfully.");
        }
        else
        {
            Serial.println("RFID Write Failed");
            tft.println("RFID Write Failed");
            request->send(500, "text/plain", "Failed to write name to RFID.");
        }
    }
    else
    {
        // Missing name parameter
        Serial.println("Form submission missing required field: name.");
        request->send(400, "text/plain", "Bad Request: Missing required field 'name'.");
    }
  });

  // Start the server
  server.begin();
}

void loop()
{
  // Read player's name from RFID
  char playerName[17]; // 16 characters + null terminator
  if (readPlayerNameFromRFID(playerName))
  {
      // Display name
      Serial.print("Player Name: ");
      Serial.println(playerName);

      // Display on TFT
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.println("Player Name:");
      tft.println(playerName);
  }
  else
  {
      Serial.println("Failed to read name from RFID.");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.println("Read Failed");
  }

  delay(1000); // Add a delay to prevent rapid looping
}