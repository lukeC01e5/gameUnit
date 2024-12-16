#ifndef OBJECT_H
#define OBJECT_H

#include <Arduino.h> // Include Arduino.h to use String
#include "RFIDData.h"

class Player
{
public:
    String name;           // Trainer Name
    uint8_t age;           // Trainer Age (1-99)
    String creatureName;   // Creature Name
    uint8_t creatureType;  // Placeholder for Creature Type
    uint16_t coin;         // Placeholder for Coin
    uint8_t environmentFlags; // Placeholder for Environment Flags

    // Default constructor
    Player() : age(1), creatureType(0), coin(0), environmentFlags(0) {}

    // Constructor from PlayerData
    Player(const PlayerData &data)
    {
        name = String(data.name);
        age = data.age;
        creatureName = String(data.creatureName);
        creatureType = data.creatureType;
        coin = data.coin;
        environmentFlags = data.environmentFlags;
    }

    // Convert to PlayerData
    PlayerData toPlayerData() const
    {
        PlayerData data;
        memset(&data, 0, sizeof(data));
        name.toCharArray(data.name, sizeof(data.name));
        data.age = age;
        creatureName.toCharArray(data.creatureName, sizeof(data.creatureName));
        data.creatureType = creatureType;
        data.coin = coin;
        data.environmentFlags = environmentFlags;
        return data;
    }
};

#endif // OBJECT_H