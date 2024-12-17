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

TFT_eSPI tft = TFT_eSPI();        // Create TFT instance
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
MFRC522::MIFARE_Key key;          // Create MIFARE_Key instance

AsyncWebServer server(80); // Create AsyncWebServer object on port 80

// WiFi credentials
const char *ssid = SECRET_SSID;
const char *pass = SECRET_PASS;

String inputData = "";       // Variable to store input data from the web page
bool dataReceived = false;   // Flag to indicate data has been received
bool tagDetected = false;    // Flag to indicate RFID tag is detected

RFIDData rfidData; // Struct to hold parsed RFID data

// Create a new SPI bus instance for RFID
SPIClass SPI_RFID(VSPI);

// Function Declarations
void listSPIFFSFiles();
void printSPIFFSInfo();
String readFromRFID(byte blockAddr);
void handleFormSubmit(AsyncWebServerRequest *request);
bool writeRFIDData(const RFIDData &data);
bool writeToRFID(const String &data, byte blockAddr);

// Function Definitions

// List all files in SPIFFS
void listSPIFFSFiles()
{
    Serial.println("Listing SPIFFS Files:");
    fs::File root = SPIFFS.open("/");
    if (!root)
    {
        Serial.println("Failed to open root directory");
        return;
    }
    fs::File file = root.openNextFile();
    while (file)
    {
        Serial.print("FILE: ");
        Serial.print(file.name());
        Serial.print("\tSIZE: ");
        Serial.println(file.size());
        file = root.openNextFile();
    }
}

// Print SPIFFS usage information
void printSPIFFSInfo()
{
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;

    Serial.println("===== SPIFFS Info =====");
    Serial.print("Total Bytes: ");
    Serial.println(totalBytes);
    Serial.print("Used Bytes: ");
    Serial.println(usedBytes);
    Serial.print("Free Bytes: ");
    Serial.println(freeBytes);
    Serial.println("=======================");
}

// Handle form submission from the web page
void handleFormSubmit(AsyncWebServerRequest *request)
{
    Serial.println("Handling form submission");

    if (request->hasParam("age", true) && request->hasParam("coins", true) &&
        request->hasParam("creatureType", true) && request->hasParam("creatureName", true))
    {
        int age = request->getParam("age", true)->value().toInt();
        int coins = request->getParam("coins", true)->value().toInt();
        int creatureType = request->getParam("creatureType", true)->value().toInt();
        String creatureName = request->getParam("creatureName", true)->value();

        RFIDData data;
        data.age = age;
        data.coins = coins;
        data.creatureType = creatureType;
        data.creatureName = creatureName;

        Serial.println("Form Data Received:");
        Serial.print("Age: ");
        Serial.println(data.age);
        Serial.print("Coins: ");
        Serial.println(data.coins);
        Serial.print("Creature Type: ");
        Serial.println(data.creatureType);
        Serial.print("Creature Name: ");
        Serial.println(data.creatureName);

        if (writeRFIDData(data))
        {
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0);
            tft.println("Data Written to RFID");
            request->send(200, "text/plain", "Data received and written to RFID card.");
        }
        else
        {
            Serial.println("Failed to write data to RFID");
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0);
            tft.println("Write Failed");
            request->send(500, "text/plain", "Failed to write data to RFID card.");
        }
    }
    else
    {
        Serial.println("Incomplete form data");
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.println("Incomplete Data");
        request->send(400, "text/plain", "Incomplete data received.");
    }
}

// Write RFIDData to the RFID card
bool writeRFIDData(const RFIDData &data)
{
    // Format data as per your RFIDData structure
    // Example: &AACCCTT%NNNN...
    String formattedData = "&";
    formattedData += (data.age < 10 ? "0" : "") + String(data.age, DEC);
    formattedData += (data.coins < 10 ? "0" : "") + String(data.coins, DEC);
    formattedData += (data.creatureType < 10 ? "0" : "") + String(data.creatureType, DEC);
    formattedData += "%";
    formattedData += data.creatureName;

    Serial.print("Formatted Data: ");
    Serial.println(formattedData);

    // Write to RFID card (e.g., block 1)
    return writeToRFID(formattedData, 1);
}

// Write a string to the RFID card at a specific block
bool writeToRFID(const String &data, byte blockAddr)
{
    MFRC522::StatusCode status;

    // Authenticate with the card (using key A)
    byte trailerBlock = (blockAddr / 4) * 4 + 3; // Trailer block for the sector

    // Default key for MIFARE cards
    for (byte i = 0; i < 6; i++)
        key.keyByte[i] = 0xFF;

    Serial.print("Authenticating with trailer block ");
    Serial.println(trailerBlock);

    // Select the card
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        Serial.println("No card selected or failed to read card serial.");
        return false;
    }

    // Authenticate
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("PCD_Authenticate() failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return false;
    }

    Serial.println("Authentication successful");

    // Prepare data (must be 16 bytes)
    byte dataBlock[16];
    memset(dataBlock, 0, sizeof(dataBlock)); // Clear the array
    data.toCharArray((char *)dataBlock, 16);

    // Write data to the card
    Serial.print("Writing to block ");
    Serial.println(blockAddr);
    status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("MIFARE_Write() failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return false;
    }

    Serial.println("Write successful");

    // Halt PICC and stop encryption on PCD
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return true;
}

// Read data from RFID card at a specific block
String readFromRFID(byte blockAddr)
{
    MFRC522::StatusCode status;
    byte trailerBlock = (blockAddr / 4) * 4 + 3; // Trailer block for the sector

    // Authenticate with the card (using key A)
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }

    Serial.print("Authenticating with trailer block ");
    Serial.println(trailerBlock);

    // Select the card
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        Serial.println("No card selected or failed to read card serial.");
        return "";
    }

    // Authenticate
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("PCD_Authenticate() failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return "";
    }

    Serial.println("Authentication successful");

    // Read data from the card
    byte buffer[18];
    byte size = sizeof(buffer);

    Serial.print("Reading from block ");
    Serial.println(blockAddr);
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("MIFARE_Read() failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return "";
    }

    Serial.println("Read successful");

    // Convert buffer to string
    String data = "";
    for (byte i = 0; i < 16; i++)
    {
        if (buffer[i] == 0)
            break; // Stop at null character
        data += (char)buffer[i];
    }

    // Halt PICC and stop encryption on PCD
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return data;
}

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
    tft.println("TFT Initialized");
    Serial.println("TFT initialized");

    // Initialize RFID module on a separate SPI bus
    Serial.println("Initializing RFID...");
    SPI_RFID.begin(25, 33, 26); // RFID SPI with SCK=25, MISO=33, MOSI=26
    mfrc522.PCD_Init();         // Initialize RFID reader
    Serial.println("RFID initialized");
    tft.println("RFID Initialized");

    // Prepare the key (used both as key A and as key B)
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }

    // Initialize SPIFFS
    Serial.println("Initializing SPIFFS...");
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        tft.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println("SPIFFS Mounted Successfully");
    tft.println("SPIFFS Ready");

    // Print SPIFFS info and list files
    printSPIFFSInfo();
    listSPIFFSFiles();

    // Connect to WiFi
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    tft.println("Connecting to WiFi...");
    WiFi.begin(ssid, pass);
    int wifi_attempts = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
        tft.print(".");
        wifi_attempts++;
        if (wifi_attempts > 20)
        { // Timeout after 20 seconds
            Serial.println("Failed to connect to WiFi");
            tft.println();
            tft.println("No WiFi Connection");
            return;
        }
    }
    Serial.println("\nConnected to WiFi");
    tft.println("\nWiFi Connected");

    // Display IP address on Serial and TFT
    String ipAddress = WiFi.localIP().toString();
    Serial.print("IP Address: ");
    Serial.println(ipAddress);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println("IP Address:");
    tft.println(ipAddress);

    // Setup server routes
    Serial.println("Setting up server routes...");
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Serving /index.html");
        request->send(SPIFFS, "/index.html", "text/html"); });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Serving /style.css");
        request->send(SPIFFS, "/style.css", "text/css"); });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        Serial.println("Serving /script.js");
        request->send(SPIFFS, "/script.js", "application/javascript"); });

    server.on("/submit", HTTP_POST, handleFormSubmit);

    server.begin();
    Serial.println("HTTP server started");
    tft.println("HTTP Server Started");
}

void loop()
{
    // The main loop remains empty as the server operates asynchronously
}