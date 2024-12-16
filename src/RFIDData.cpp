// RFIDData.cpp
#include "RFID.h"
#include "RFIDData.h"
#include <MFRC522.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// Define the MFRC522 instance with the correct pins
MFRC522 mfrc522(SS_PIN_RFID, RST_PIN_RFID);

// Define the MIFARE_Key instance
MFRC522::MIFARE_Key key;

// PlayerData should be a multiple of 16 bytes (size of a MIFARE block)

/*
struct PlayerData
{
    char name[16];         // 16 bytes
    uint8_t age;           // 1 byte
    char creatureName[16]; // 16 bytes
    // Additional fields...
    uint8_t padding[13]; // Adjust padding to make total size a multiple of 16
};
*/
// Implement the initializeRFID function
void initializeRFID()
{
    Serial.println("Initializing RFID...");
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN); // Initialize SPI bus with defined pins
    mfrc522.PCD_Init();                     // Initialize the RFID reader
    Serial.println("RFID initialized");

    // Optionally, display on TFT
    extern TFT_eSPI tft; // Declare TFT instance
    tft.println("RFID OK");

    // Prepare the key (used both as Key A and Key B)
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }
}

bool writePlayerDataToRFID(const PlayerData &data)
{
    // Authenticate with the card
    MFRC522::StatusCode status;

    // Select the card
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        Serial.println("No card selected or failed to read card serial.");
        return false;
    }

    // Calculate the number of blocks needed
    const int dataSize = sizeof(PlayerData);
    const int blocksNeeded = (dataSize + 15) / 16; // Each block is 16 bytes

    // Write to multiple blocks starting from block 1 (avoid block 0)
    for (int i = 0; i < blocksNeeded; ++i)
    {
        byte blockAddr = 1 + i;

        // Authenticate for each sector
        byte sector = blockAddr / 4;
        byte currentTrailerBlock = sector * 4 + 3;
        status = mfrc522.PCD_Authenticate(
            MFRC522::PICC_CMD_MF_AUTH_KEY_A,
            currentTrailerBlock,
            &key,
            &(mfrc522.uid));

        Serial.print("Authenticating block ");
        Serial.print(blockAddr);
        Serial.print(": ");
        Serial.println(mfrc522.GetStatusCodeName(status));

        if (status != MFRC522::STATUS_OK)
        {
            Serial.print("PCD_Authenticate() failed for block ");
            Serial.print(blockAddr);
            Serial.print(": ");
            Serial.println(mfrc522.GetStatusCodeName(status));
            return false;
        }

        // Prepare data block
        byte buffer[16];
        memset(buffer, 0, sizeof(buffer));
        int bytesToCopy = min(16, dataSize - (i * 16));
        memcpy(buffer, ((byte *)&data) + (i * 16), bytesToCopy);

        // Write data block
        status = mfrc522.MIFARE_Write(blockAddr, buffer, 16);
        if (status != MFRC522::STATUS_OK)
        {
            Serial.print("MIFARE_Write() failed for block ");
            Serial.print(blockAddr);
            Serial.print(": ");
            Serial.println(mfrc522.GetStatusCodeName(status));
            mfrc522.PCD_StopCrypto1();
            return false;
        }

        // Stop encryption for this block
        mfrc522.PCD_StopCrypto1();
    }

    Serial.println("Write successful");

    // Halt PICC and stop Crypto1
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return true;
}

bool readPlayerDataFromRFID(PlayerData &data)
{
    // Authenticate with the card
    MFRC522::StatusCode status;

    // Select the card
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        Serial.println("No card selected or failed to read card serial.");
        return false;
    }

    const int dataSize = sizeof(PlayerData);
    const int blocksNeeded = (dataSize + 15) / 16; // Each block is 16 bytes

    // Initialize a buffer to hold all data
    byte buffer[dataSize];
    memset(buffer, 0, sizeof(buffer));

    for (int i = 0; i < blocksNeeded; ++i)
    {
        byte blockAddr = 1 + i;

        // Authenticate for each sector
        byte sector = blockAddr / 4;
        byte currentTrailerBlock = sector * 4 + 3;
        status = mfrc522.PCD_Authenticate(
            MFRC522::PICC_CMD_MF_AUTH_KEY_A,
            currentTrailerBlock,
            &key,
            &(mfrc522.uid));

        Serial.print("Authenticating block ");
        Serial.print(blockAddr);
        Serial.print(": ");
        Serial.println(mfrc522.GetStatusCodeName(status));

        if (status != MFRC522::STATUS_OK)
        {
            Serial.print("PCD_Authenticate() failed for block ");
            Serial.print(blockAddr);
            Serial.print(": ");
            Serial.println(mfrc522.GetStatusCodeName(status));
            return false;
        }

        // Read data block
        byte blockBuffer[18]; // 16 bytes data + 2 bytes CRC
        byte size = sizeof(blockBuffer);
        status = mfrc522.MIFARE_Read(blockAddr, blockBuffer, &size);

        Serial.print("Reading block ");
        Serial.print(blockAddr);
        Serial.print(": ");
        Serial.println(mfrc522.GetStatusCodeName(status));

        if (status != MFRC522::STATUS_OK)
        {
            Serial.print("MIFARE_Read() failed for block ");
            Serial.print(blockAddr);
            Serial.print(": ");
            Serial.println(mfrc522.GetStatusCodeName(status));
            mfrc522.PCD_StopCrypto1();
            return false;
        }

        // Copy data to the main buffer
        int bytesToCopy = min(16, dataSize - (i * 16));
        memcpy(buffer + (i * 16), blockBuffer, bytesToCopy);
    }

    // Assign buffer to PlayerData
    memcpy(&data, buffer, sizeof(PlayerData));

    Serial.println("Read successful");

    // Halt PICC and stop Crypto1
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return true;
}

bool writeSimpleDataToRFID(const byte *data, byte dataSize)
{
    // Implement if required
    return false;
}

bool readSimpleDataFromRFID(byte *buffer, byte bufferSize)
{
    // Implement if required
    return false;
}

void parseRFIDData(const String &data, PlayerData &playerData)
{
    // Function implementation...

    // Example parsing logic (ensure members match PlayerData)
    int startIndex = 0;
    int endIndex = data.indexOf(',');

    // Parse Name
    if (endIndex > startIndex)
    {
        data.substring(startIndex, endIndex).toCharArray(playerData.name, sizeof(playerData.name));
    }
    else
    {
        playerData.name[0] = '\0'; // Empty string
    }

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);

    // Parse Age
    if (endIndex > startIndex)
    {
        playerData.age = data.substring(startIndex, endIndex).toInt();
    }
    else
    {
        playerData.age = 0; // Default age
    }

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);

    // Parse Creature Name
    if (endIndex > startIndex)
    {
        data.substring(startIndex, endIndex).toCharArray(playerData.creatureName, sizeof(playerData.creatureName));
    }
    else
    {
        playerData.creatureName[0] = '\0'; // Empty string
    }

    // Continue parsing other fields similarly
    // For placeholders, assign default values or parse if implemented
    // ...
}

// Write only the player's name to block 1
bool writePlayerNameToRFID(const char *name)
{
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        // Serial.println("No card present or could not read card serial.");
        // tft.println("No card detected.");
        return false;
    }

    // Authenticate with key A
    MFRC522::StatusCode status;
    byte blockAddr = 1;
    byte sector = blockAddr / 4;
    byte trailerBlock = sector * 4 + 3;
    status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        trailerBlock,
        &key,
        &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Authentication failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Prepare data buffer
    byte buffer[16];
    memset(buffer, 0, sizeof(buffer));
    strncpy((char *)buffer, name, 16);

    // Write data to block 1
    status = mfrc522.MIFARE_Write(blockAddr, buffer, 16);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Write failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PCD_StopCrypto1();
        return false;
    }

    Serial.println("Name written successfully.");
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return true;
}

// Read only the player's name from block 1
bool readPlayerNameFromRFID(char *name)
{
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    {
        // Serial.println("No card present or could not read card serial.");
        // tft.println("No card detected.");
        return false;
    }

    // Authenticate with key A
    MFRC522::StatusCode status;
    byte blockAddr = 1;
    byte sector = blockAddr / 4;
    byte trailerBlock = sector * 4 + 3;
    status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A,
        trailerBlock,
        &key,
        &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Authentication failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Read data from block 1
    byte buffer[18];
    byte size = sizeof(buffer);
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
        Serial.print("Read failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        mfrc522.PCD_StopCrypto1();
        return false;
    }

    // Copy name to output buffer
    strncpy(name, (char *)buffer, 16);
    name[16] = '\0'; // Ensure null-termination

    Serial.println("Name read successfully.");
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return true;
}