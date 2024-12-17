#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <MFRC522.h>
#include "RFIDData.h"

// Include the file with the WiFi credentials
#include "arduino_secrets.h"

// Pin definitions for the RFID module
#define SS_PIN 13  // Slave Select pin for RFID
#define RST_PIN 22 // Reset pin for RFID

// Initialize SPI with specific pins (SCK=25, MISO=33, MOSI=26)
void setupSPI()
{
    SPI.begin(25, 33, 26);
}

// Create instances
TFT_eSPI tft = TFT_eSPI();     // Initialize TFT display
MFRC522 rfid(SS_PIN, RST_PIN); // Initialize MFRC522 RFID reader
MFRC522::MIFARE_Key key;       // MIFARE key for authentication

AsyncWebServer server(80); // Create AsyncWebServer object on port 80

// WiFi credentials
const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;

// Creature list
const char *creatures[] = {
    "None", "Flamingo", "Flame-Kingo", "Kitten", "Flame-on", "Pup", "Dog", "Wolf",
    "Birdy", "Haast-eagle", "Squidy", "Giant-Squid", "Kraken", "BabyShark", "Shark",
    "Megalodon", "Tadpole", "Poison-dart-frog", "Unicorn", "Master-unicorn", "Sprouty",
    "Tree-Folk", "Bush-Monster", "Baby-Dragon", "Dragon", "Dino-Egg", "T-Rex", "Baby-Ray",
    "Mega-Manta", "Orca", "Big-Bitey", "Flame-Lily", "Monster-Lily", "Bear-Cub", "Moss-Bear"};

void handleFormSubmit(AsyncWebServerRequest *request);
bool writeRFIDData(const RFIDData &data);
bool readRFIDData(RFIDData &data);

void setup()
{
    // Initialize Serial Monitor
    Serial.begin(115200);

    // Initialize TFT display
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println("Initializing...");

    // Initialize SPI
    setupSPI();

    // Initialize RFID reader
    rfid.PCD_Init();
    tft.println("RFID Ready");

    // Prepare MIFARE Key
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }

    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("Error mounting SPIFFS");
        tft.println("SPIFFS Mount Failed");
        return;
    }
    tft.println("SPIFFS Ready");

    // Connect to WiFi
    tft.println("Connecting to WiFi...");
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        tft.print(".");
    }
    tft.println("\nWiFi Connected");
    tft.println("IP: " + WiFi.localIP().toString());

    // Setup server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });

    server.on("/submit", HTTP_POST, handleFormSubmit);

    server.begin();
    tft.println("Server Started");
}

void loop()
{
    RFIDData data;
    if (readRFIDData(data))
    {
        Serial.println("RFID Read Successful");
        Serial.println("Age: " + String(data.age));
        Serial.println("Coins: " + String(data.coins));
        Serial.println("Creature Type: " + String(data.creatureType));
        Serial.println("Creature Name: " + data.creatureName);

        // Update TFT display
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.println("Player Info:");
        tft.println("Age: " + String(data.age));
        tft.println("Coins: " + String(data.coins));
        if (data.creatureType >= 0 && data.creatureType <= 34)
        {
            tft.println("Creature: " + String(creatures[data.creatureType]));
        }
        else
        {
            tft.println("Creature: Unknown");
        }
        tft.println("Name: " + data.creatureName);
    }
    else
    {
        // Display message
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.println("No RFID Tag Detected");
    }

    delay(1000);
}

// Handle form submission
void handleFormSubmit(AsyncWebServerRequest *request)
{
    if (request->method() != HTTP_POST)
    {
        request->send(405, "text/plain", "Method Not Allowed");
        return;
    }

    RFIDData rfidData;

    // Extract form parameters
    if (request->hasParam("age", true))
    {
        rfidData.age = request->getParam("age", true)->value().toInt();
    }
    if (request->hasParam("coins", true))
    {
        rfidData.coins = request->getParam("coins", true)->value().toInt();
    }
    if (request->hasParam("creatureType", true))
    {
        rfidData.creatureType = request->getParam("creatureType", true)->value().toInt();
    }
    if (request->hasParam("creatureName", true))
    {
        rfidData.creatureName = request->getParam("creatureName", true)->value();
    }

    // Write data to RFID
    if (writeRFIDData(rfidData))
    {
        Serial.println("RFID Write Successful");
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.println("RFID Write Success");
        request->send(200, "text/plain", "Data written to RFID successfully.");
    }
    else
    {
        Serial.println("RFID Write Failed");
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.println("RFID Write Failed");
        request->send(500, "text/plain", "Failed to write data to RFID.");
    }
}

// Function to write data to the RFID tag
bool writeRFIDData(const RFIDData &data)
{
    MFRC522::StatusCode status;

    // Check for new card
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    {
        Serial.println("No card detected");
        return false;
    }

    byte blockAddr = 1;
    byte trailerBlock = (blockAddr / 4) * 4 + 3;

    // Authenticate
    status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(rfid.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Authentication failed: ");
        Serial.println(rfid.GetStatusCodeName(status));
        return false;
    }

    // Prepare data
    // Data format: &AACCCTT%NNNN...
    // AA: Age, CC: Coins, TT: Creature Type, %: Separator, NNNN...: Creature Name
    String ageStr = String(data.age);
    if (ageStr.length() < 2)
        ageStr = "0" + ageStr;

    String coinsStr = String(data.coins);
    if (coinsStr.length() < 2)
        coinsStr = "0" + coinsStr;

    String creatureTypeStr = String(data.creatureType);
    if (creatureTypeStr.length() < 2)
        creatureTypeStr = "0" + creatureTypeStr;

    String serializedData = "&" + ageStr + coinsStr + creatureTypeStr + "%" + data.creatureName;

    // Ensure the serialized data does not exceed block size
    if (serializedData.length() > 16)
    {
        serializedData = serializedData.substring(0, 16);
    }

    byte dataBlock[16];
    memset(dataBlock, 0, sizeof(dataBlock));
    serializedData.toCharArray((char *)dataBlock, 16);

    // Write to block
    status = rfid.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Write failed: ");
        Serial.println(rfid.GetStatusCodeName(status));
        rfid.PCD_StopCrypto1();
        return false;
    }

    // Stop authentication
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    return true;
}

// Function to read data from the RFID tag
bool readRFIDData(RFIDData &data)
{
    MFRC522::StatusCode status;

    // Check for new card
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    {
        return false;
    }

    byte blockAddr = 1;
    byte trailerBlock = (blockAddr / 4) * 4 + 3;

    // Authenticate
    status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(rfid.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Authentication failed: ");
        Serial.println(rfid.GetStatusCodeName(status));
        rfid.PCD_StopCrypto1();
        return false;
    }

    // Read block
    byte buffer[18];
    byte size = sizeof(buffer);

    status = rfid.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Read failed: ");
        Serial.println(rfid.GetStatusCodeName(status));
        rfid.PCD_StopCrypto1();
        return false;
    }

    // Convert buffer to string
    String serializedData = "";
    for (byte i = 0; i < 16; i++)
    {
        if (buffer[i] != 0)
            serializedData += (char)buffer[i];
    }

    // Parse the data
    parseRFIDData(serializedData, data);

    // Stop authentication
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    return true;
}