#include "Creature.h"

// Forward-declare your creatures array, or include a header that declares it.
extern const char *creatures[35]; // Must match the array in main.cpp

/*
  Expected format: "AA CC TT BB%NAME"
   - AA: age (2 chars)
   - CC: coins (2 chars)
   - TT: creature type (2 chars)
   - BB: bits (2 chars, optional)
   - %NAME: up to 6 chars
*/
// ...existing includes...
/*
Creature decode(int numericPart, const String &namePart)
{
    Creature c;

    // Convert numericPart to an 8-digit string (leading zeros included)
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%08d", numericPart);
    String mainData = buffer;

    // Expect mainData to have 8 chars => [0..1] age, [2..3] coins, [4..5] creatureType, [6..7] boolVal
    int lengthNeeded = 8;
    if (mainData.length() < lengthNeeded)
    {
        Serial.println("decode: Not enough digits in numericPart");
        return c; // return an empty Creature
    }

    // Parse fields
    c.trainerAge = mainData.substring(0, 2).toInt();
    c.coins = mainData.substring(2, 4).toInt();
    c.creatureType = mainData.substring(4, 6).toInt();
    // If your Creature struct has a boolVal or bools, parse it here:
    // int boolVal    = mainData.substring(6, 8).toInt();

    // Assign the name, up to 6 chars
    if (namePart.length() > 6)
    {
        c.customName = namePart.substring(0, 6);
    }
    else
    {
        c.customName = namePart;
    }

    // Convert numeric creatureType to a text creatureName (if in range)
    if (c.creatureType >= 0 && c.creatureType < 35)
    {
        c.creatureName = creatures[c.creatureType];
    }
    else
    {
        c.creatureName = "Unknown Creature";
    }

    // Remove any embedded nulls
    int nullPos;
    while ((nullPos = c.customName.indexOf('\0')) != -1)
    {
        c.customName.remove(nullPos, 1);
    }

    // (Optional) Trim whitespace
    c.customName.trim();

    // Debug output
    Serial.println("[decode] Created Creature object from numericPart & namePart:");
    Serial.print("  Age: ");
    Serial.println(c.trainerAge);
    Serial.print("  Coins: ");
    Serial.println(c.coins);
    Serial.print("  creatureType: ");
    Serial.println(c.creatureType);
    Serial.print("  creatureName: ");
    Serial.println(c.creatureName);
    Serial.print("  customName: ");
    Serial.println(c.customName);

    return c;
}
*/