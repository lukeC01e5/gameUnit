#ifndef RFIDDATA_H
#define RFIDDATA_H

#include <Arduino.h>

struct RFIDData
{
    int age;              // Age (00-99)
    int coins;            // Coins (00-99)
    int creatureType;     // Creature Type (00-34)
    String creatureName;  // User's creature name
};

void parseRFIDData(const String &data, RFIDData &rfidData);

#endif // RFIDDATA_H