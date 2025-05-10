#include <Keypad.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <MFRC522.h>
#include "RFIDData.h"
#include "arduino_secrets.h"
#include "GlobalDefs.h" // SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN, RST_PIN, key, mfrc522, etc.

// Keypad configuration (4x3 keypad)
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
byte rowPins[ROWS] = {21, 17, 2, 15};
byte colPins[COLS] = {12, 27, 32};

Keypad keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
TFT_eSPI tft; // TFT display instance

// Forward declarations
void add_5_coin(const String &rfidUID);
String readBlock1();
bool writeBlock1(const String &data);
void debugReadSector0();
String getChallengeCodeFromKeypad();
bool checkForUser(const String &rfidUID);
void cashInLoot();
Character parseTagData(const String &raw);
String getRfidUidString();
void challengeUpdate(const Character &character);

////////////////////////////////////////////////////////////////////////////////
// Helper to build a hex-string from the card’s UID
String getRfidUidString()
{
    Serial.println("[getRfidUidString] Converting card UID to String...");
    String uidStr = "";
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        // Debug each UID byte
        Serial.print("  [getRfidUidString] UID byte ");
        Serial.print(i);
        Serial.print(": 0x");
        Serial.println(mfrc522.uid.uidByte[i], HEX);

        if (mfrc522.uid.uidByte[i] < 0x10)
        {
            uidStr += "0";
        }
        uidStr += String(mfrc522.uid.uidByte[i], HEX);
    }
    uidStr.toUpperCase();
    Serial.println("[getRfidUidString] Final UID string: " + uidStr);
    return uidStr;
}

////////////////////////////////////////////////////////////////////////////////
// Add a helper function to update challenge data on the database.
void challengeUpdate(const Character &character)
{
    Serial.println("[challengeUpdate] Entering function.");
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[challengeUpdate] WiFi not connected - aborting.");
        return;
    }

    Serial.println("[challengeUpdate] WiFi connected. Starting HTTP POST request...");

    WiFiClient wifiClient;
    HttpClient http(wifiClient, "gameapi-2e9bb6e38339.herokuapp.com", 80);

    // Build JSON payload
    String payload = "{";
    payload += "\"customName\":\"" + character.customName + "\",";
    payload += "\"challengeCode\":" + String(character.challengeCode) + ",";
    payload += "\"wrongGuesses\":" + String(character.wrongGuesses);
    payload += "}";

    Serial.println("[challengeUpdate] Payload to send: " + payload);

    if (wifiClient.connect("gameapi-2e9bb6e38339.herokuapp.com", 80))
    {
        Serial.println("[challengeUpdate] Connected to server, sending request...");
        http.beginRequest();
        http.post("/api/v1/challenge_update");
        http.sendHeader("Content-Type", "application/json");
        http.sendHeader("Content-Length", payload.length());
        http.beginBody();
        http.print(payload);
        http.endRequest();

        int statusCode = http.responseStatusCode();
        String response = http.responseBody();
        Serial.print("[challengeUpdate] statusCode: ");
        Serial.println(statusCode);
        Serial.print("[challengeUpdate] responseBody: ");
        Serial.println(response);

        wifiClient.stop();
    }
    else
    {
        Serial.println("[challengeUpdate] Connection to server failed.");
    }
    Serial.println("[challengeUpdate] Exiting function.");
}

////////////////////////////////////////////////////////////////////////////////
// Parse "CCCW?BB%Name" into a Character object
Character parseTagData(const String &raw)
{
    Serial.println("[parseTagData] Entering function. Raw data: " + raw);

    Character c;
    c.coins = 0;
    if (raw.indexOf('?') == -1 || raw.indexOf('%') == -1)
    {
        Serial.println("[parseTagData] Data invalid. Missing '?' or '%'. Returning empty.");
        return c;
    }

    // First 3 -> challengeCode
    c.challengeCode = raw.substring(0, 3).toInt();
    // 4th char -> wrong guesses
    c.wrongGuesses = raw.substring(3, 4).toInt();

    int qIndex = raw.indexOf('?');
    int pIndex = raw.indexOf('%');

    // boolVal -> the substring between '?' and '%'
    String boolValStr = raw.substring(qIndex + 1, pIndex);
    c.boolVal = boolValStr.toInt();

    // name -> substring after '%'
    c.customName = raw.substring(pIndex + 1);

    Serial.print("[parseTagData] challengeCode=");
    Serial.println(c.challengeCode);
    Serial.print("[parseTagData] wrongGuesses=");
    Serial.println(c.wrongGuesses);
    Serial.print("[parseTagData] boolVal=");
    Serial.println(c.boolVal);
    Serial.print("[parseTagData] customName=");
    Serial.println(c.customName);

    Serial.println("[parseTagData] Exiting function. Returning Character.");
    return c;
}

////////////////////////////////////////////////////////////////////////////////
// Function to handle "cash in loot" flow
void cashInLoot()
{
    Serial.println("[cashInLoot] Entering function...");

    // Connect to Wi-Fi if needed
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[cashInLoot] WiFi not connected. Attempting to connect or skipping...");
        // Possibly you do a WiFi.begin(...) here if desired
    }

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println("Cash in loot...");
    Serial.println("[cashInLoot] Prompting user to press RFID tag...");

    // Wait for RFID tag
    while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        delay(100);
    }
    Serial.println("[cashInLoot] Card detected. Authenticating...");

    // Authenticate and read block 1
    {
        byte trailerBlock = 3;
        MFRC522::StatusCode authStatus = mfrc522.PCD_Authenticate(
            MFRC522::PICC_CMD_MF_AUTH_KEY_A,
            trailerBlock,
            &key,
            &(mfrc522.uid));
        if (authStatus != MFRC522::STATUS_OK)
        {
            Serial.print("[cashInLoot] Auth failed. Status: ");
            Serial.println(mfrc522.GetStatusCodeName(authStatus));
            Serial.println("[cashInLoot] Aborting cashInLoot.");
            return;
        }
        else
        {
            Serial.println("[cashInLoot] Auth successful. Reading block 1...");
        }
    }

    // Attempt to read block 1
    String block1Data = readBlock1();
    Serial.println("[cashInLoot] block1Data: " + block1Data);

    // Check presence of '?' and '%'
    if (block1Data.indexOf('?') == -1 || block1Data.indexOf('%') == -1)
    {
        Serial.println("[cashInLoot] Invalid data or missing markers. Aborting.");
        return;
    }

    // Parse it into a Character
    Character c = parseTagData(block1Data);

    // Convert MFRC522’s UID to a String
    String rfidUID = getRfidUidString();
    Serial.println("[cashInLoot] RFID UID is: " + rfidUID);

    // If boolVal==15 => add coins + challengeUpdate => reset boolVal => write
    if (c.boolVal == 15)
    {
        Serial.println("[cashInLoot] boolVal == 15, so we add coin and update challenge...");
        add_5_coin(rfidUID);
        challengeUpdate(c);

        // Reset boolVal to 00, then write back
        Serial.println("[cashInLoot] Resetting boolVal to 00...");
        char ccBuf[4];
        snprintf(ccBuf, sizeof(ccBuf), "%03d", c.challengeCode);
        char wgBuf[2];
        snprintf(wgBuf, sizeof(wgBuf), "%1d", c.wrongGuesses);

        String newRaw = String(ccBuf) + String(wgBuf) + "?00%" + c.customName;
        Serial.println("[cashInLoot] Writing new data to block1: " + newRaw);

        bool writeOk = writeBlock1(newRaw);
        if (writeOk)
        {
            Serial.println("[cashInLoot] boolVal reset to 00 on tag (write success).");
        }
        else
        {
            Serial.println("[cashInLoot] Failed to write new data to block1.");
        }
    }
    else
    {
        Serial.print("[cashInLoot] boolVal is ");
        Serial.print(c.boolVal);
        Serial.println(" - skipping coin addition.");
    }

    // Halt card
    mfrc522.PICC_HaltA();
    Serial.println("[cashInLoot] Done. Exiting function.");
}

////////////////////////////////////////////////////////////////////////////////
// Setup
void setup()
{
    Serial.begin(115200);
    Serial.println("[setup] Starting Setup...");

    // Initialize TFT
    tft.init();
    tft.setRotation(3);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println("TFT Initialized");
    Serial.println("[setup] TFT initialized.");

    Serial.println("[setup] Initializing SPI...");
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    Serial.println("[setup] SPI started.");

    Serial.println("[setup] Initializing RFID (mfrc522)...");
    mfrc522.PCD_Init();
    Serial.println("[setup] RFID initialization complete.");

    // Set default global key (0xFF...FF)
    Serial.println("[setup] Setting default key to 0xFF for all 6 bytes.");
    for (int i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }

    // Show user options on TFT
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Press 1 to Cash Loot");
    tft.println("Press 2 to Challenge");
    Serial.println("[setup] Prompting user with 2 options...");

    // Wait until keypad press
    char inputKey = NO_KEY;
    while (inputKey == NO_KEY)
    {
        inputKey = keypad.getKey();
        delay(100);
    }
    Serial.print("[setup] User pressed: ");
    Serial.println(inputKey);

    if (inputKey == '1')
    {
        Serial.println("[setup] Going to cashInLoot()");
        cashInLoot();
    }
    else if (inputKey == '2')
    {
        Serial.println("[setup] Going to normal challenge flow in loop()");
    }
    else
    {
        Serial.println("[setup] Unrecognized input - ignoring.");
    }

    tft.fillScreen(TFT_BLACK);
    Serial.println("[setup] Setup complete.\n");
}

////////////////////////////////////////////////////////////////////////////////
// Main loop
void loop()
{
    Serial.println("\n[loop] New iteration...");

    // 1) Get 3-digit code
    String newChallengeCode = getChallengeCodeFromKeypad();
    Serial.print("[loop] newChallengeCode is: ");
    Serial.println(newChallengeCode);

    tft.setCursor(0, 0);
    tft.fillScreen(TFT_BLACK);
    tft.println("\nGot: " + newChallengeCode);

    // 2) Present RFID tag
    tft.println("\nPresent RFID Tag...");
    Serial.println("[loop] Waiting for RFID tag...");
    while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        delay(100);
    }
    tft.println("Tag present!");
    Serial.println("[loop] RFID tag detected. Authenticating...");
    delay(200);

    // Authenticate before readBlock1()
    {
        byte trailerBlock = 3;
        MFRC522::StatusCode authStatus = mfrc522.PCD_Authenticate(
            MFRC522::PICC_CMD_MF_AUTH_KEY_A,
            trailerBlock,
            &key,
            &(mfrc522.uid));
        if (authStatus != MFRC522::STATUS_OK)
        {
            Serial.print("[loop] Auth for readBlock1 failed: ");
            Serial.println(mfrc522.GetStatusCodeName(authStatus));
            Serial.println("[loop] Skipping read...");
        }
        else
        {
            Serial.println("[loop] Auth success for readBlock1!");
        }
    }

    // 3) Read block 1 (no direct auth inside readBlock1())
    String block1Data = readBlock1();
    tft.println("Block 1 Data:");
    tft.println(block1Data);
    Serial.println("[loop] block1Data: " + block1Data);

    // 4) If we see '%', we update
    if (block1Data.indexOf('%') != -1 && block1Data.length() >= 4)
    {
        Serial.println("[loop] Found '%' in block1Data, building updated string...");
        String updated = newChallengeCode + block1Data.substring(3);
        Serial.println("[loop] Updated data: " + updated);

        bool result = writeBlock1(updated);
        if (result)
        {
            tft.println("Block 1 Update Succeeded!");
            Serial.println("[loop] Block 1 update succeeded!");
        }
        else
        {
            tft.println("Block 1 Update Failed!");
            Serial.println("[loop] Block 1 update failed!");
        }
    }
    else
    {
        tft.println("Invalid data format.\nNo update performed.");
        Serial.println("[loop] No '%' or data too short - skipping update.");
    }

    // Re-select the card before reading again
    delay(100);
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        Serial.println("[loop] Could not re-select card for sector read. Skipping debugReadSector0.");
    }
    else
    {
        debugReadSector0();
    }

    // Halt the card
    mfrc522.PICC_HaltA();
    Serial.println("[loop] Card halted. Loop iteration over.");
    delay(2000);
}

////////////////////////////////////////////////////////////////////////////////
// readBlock1: minimal direct read from block 1
String readBlock1()
{
    Serial.println("[readBlock1] Entering function...");
    byte buffer[18];
    byte size = sizeof(buffer);

    MFRC522::StatusCode status = mfrc522.MIFARE_Read(1, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("[readBlock1] Read failed; status: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return "";
    }

    Serial.print("[readBlock1] Raw bytes: ");
    for (byte i = 0; i < size; i++)
    {
        Serial.print(buffer[i]);
        Serial.print(" ");
    }
    Serial.println();

    String result;
    for (byte i = 0; i < 16; i++)
    {
        if (buffer[i] == 0)
            break; // Stop at null
        result += (char)buffer[i];
    }
    Serial.println("[readBlock1] Returning: " + result);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// writeBlock1: minimal direct write to block 1
bool writeBlock1(const String &newData)
{
    Serial.println("[writeBlock1] Entering function...");
    Serial.println("[writeBlock1] Data to write: " + newData);

    byte blockAddr = 1;
    MFRC522::StatusCode status;

    // Re-authenticate
    byte trailerBlock = 3;
    status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        trailerBlock,
        &key,
        &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("[writeBlock1] Auth failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PCD_StopCrypto1();
        return false;
    }

    // Prepare a 16-byte buffer
    byte buffer[16];
    memset(buffer, 0, sizeof(buffer));

    // Copy data into buffer (truncate if needed)
    int len = min(16, (int)newData.length());
    Serial.print("[writeBlock1] Copying ");
    Serial.print(len);
    Serial.println(" bytes to buffer...");
    for (int i = 0; i < len; i++)
    {
        buffer[i] = newData[i];
    }

    // Perform the actual write
    status = mfrc522.MIFARE_Write(blockAddr, buffer, 16);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("[writeBlock1] Write failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PCD_StopCrypto1();
        return false;
    }

    Serial.println("[writeBlock1] Write success!");
    mfrc522.PCD_StopCrypto1();
    Serial.println("[writeBlock1] Exiting function...");
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// debugReadSector0: Show raw data on sector 0 if needed
void debugReadSector0()
{
    Serial.println("[debugReadSector0] Entering function - custom debug code if needed.");
    // Add code to read other blocks/sectors for debugging
    // ...
    Serial.println("[debugReadSector0] Exiting function.");
}

////////////////////////////////////////////////////////////////////////////////
// getChallengeCodeFromKeypad: ask user for a 3-digit code via keypad
String getChallengeCodeFromKeypad()
{
    Serial.println("[getChallengeCodeFromKeypad] Asking for 3-digit code...");
    String code;
    tft.println("Enter 3-digit code:");

    while (code.length() < 3)
    {
        char keyP = keypad.getKey();
        if (keyP != NO_KEY && isDigit(keyP))
        {
            code += keyP;
            tft.print(keyP);
            Serial.print("[getChallengeCodeFromKeypad] Pressed: ");
            Serial.println(keyP);
        }
    }
    Serial.println("[getChallengeCodeFromKeypad] Final code: " + code);
    return code;
}

////////////////////////////////////////////////////////////////////////////////
// checkForUser: confirm user presence in DB
bool checkForUser(const String &rfidUID)
{
    Serial.println("[checkForUser] Entering function...");
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[checkForUser] WiFi not connected - returning false.");
        return false;
    }

    Serial.println("[checkForUser] Checking user with rfidUID=" + rfidUID);

    WiFiClient wifiClient;
    HttpClient http(wifiClient, "gameapi-2e9bb6e38339.herokuapp.com", 80);

    http.beginRequest();
    http.get("/api/v1/users"); // Example endpoint
    http.sendHeader("Content-Type", "application/json");
    http.endRequest();

    int statusCode = http.responseStatusCode();
    String response = http.responseBody();

    Serial.print("[checkForUser] statusCode=");
    Serial.println(statusCode);
    Serial.print("[checkForUser] response=");
    Serial.println(response);

    if (statusCode > 0)
    {
        // If the response contains rfidUID, we consider user to exist
        if (response.indexOf("\"" + rfidUID + "\"") != -1)
        {
            Serial.println("[checkForUser] User found!");
            return true;
        }
        else
        {
            Serial.println("[checkForUser] New user (not found)!");
            return false;
        }
    }
    else
    {
        Serial.print("[checkForUser] Status code error: ");
        Serial.println(statusCode);
    }

    wifiClient.stop();
    Serial.println("[checkForUser] Exiting function with false.");
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// add_5_coin: calls the server to add 5 coins to an existing user
void add_5_coin(const String &rfidUID)
{
    Serial.println("[add_5_coin] Entering function. Checking if user exists...");

    if (!checkForUser(rfidUID))
    {
        Serial.println("[add_5_coin] User not found or WiFi error - cannot add coins.");
        return;
    }

    Serial.println("[add_5_coin] User is confirmed to exist. Checking WiFi again...");
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[add_5_coin] WiFi not connected - aborting add_5_coin.");
        return;
    }

    Serial.println("[add_5_coin] Posting to /api/v1/add_5_coin with rfidUID=" + rfidUID);
    WiFiClient wifiClient;
    HttpClient http(wifiClient, "gameapi-2e9bb6e38339.herokuapp.com", 80);

    // Build JSON payload
    String payload = "{";
    payload += "\"rfidUID\":\"" + rfidUID + "\"";
    payload += "}";

    if (wifiClient.connect("gameapi-2e9bb6e38339.herokuapp.com", 80))
    {
        Serial.println("[add_5_coin] Connected to server. Sending POST...");
        http.beginRequest();
        http.post("/api/v1/add_5_coin");
        http.sendHeader("Content-Type", "application/json");
        http.sendHeader("Content-Length", payload.length());
        http.beginBody();
        http.print(payload);
        http.endRequest();

        int statusCode = http.responseStatusCode();
        String response = http.responseBody();
        Serial.print("[add_5_coin] statusCode=");
        Serial.println(statusCode);
        Serial.print("[add_5_coin] response=");
        Serial.println(response);

        wifiClient.stop();
    }
    else
    {
        Serial.println("[add_5_coin] Connection failed - cannot POST.");
    }
    Serial.println("[add_5_coin] Exiting function.");
}