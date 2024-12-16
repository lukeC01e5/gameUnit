#ifndef RFIDDATA_H
#define RFIDDATA_H

#include <Arduino.h>

// Existing struct and function declarations
struct PlayerData
{
    char name[16];            // 16 bytes
    uint8_t age;              // 1 byte
    char creatureName[16];    // 16 bytes
    uint8_t creatureType;     // 1 byte
    uint16_t coin;            // 2 bytes
    uint8_t environmentFlags; // 1 byte
    uint8_t reserved[11];     // Padding for 48 bytes total
};

bool writePlayerDataToRFID(const PlayerData &data);
bool readPlayerDataFromRFID(PlayerData &data);
void parseRFIDData(const String &data, PlayerData &playerData);

// **Add these declarations**
bool writePlayerNameToRFID(const char *name);
bool readPlayerNameFromRFID(char *name);

#endif // RFIDDATA_H