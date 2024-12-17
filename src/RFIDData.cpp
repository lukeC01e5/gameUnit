#include "RFIDData.h"

void parseRFIDData(const String &data, RFIDData &rfidData)
{
    // Data format: &AACCCTT%NNNN...
    // AA: Age, CC: Coins, TT: Creature Type, %: Separator, NNNN...: Creature Name

    if (data.length() < 7 || data.charAt(0) != '&')
    {
        // Invalid data format
        Serial.println("Invalid RFID data format");
        return;
    }

    rfidData.age = data.substring(1, 3).toInt();
    rfidData.coins = data.substring(3, 5).toInt();
    rfidData.creatureType = data.substring(5, 7).toInt();

    int nameStartIndex = data.indexOf('%') + 1;
    if (nameStartIndex > 0 && nameStartIndex < data.length())
    {
        rfidData.creatureName = data.substring(nameStartIndex);
    }
    else
    {
        rfidData.creatureName = "";
    }

    Serial.println("Parsed RFID Data:");
    Serial.print("Age: ");
    Serial.println(rfidData.age);
    Serial.print("Coins: ");
    Serial.println(rfidData.coins);
    Serial.print("Creature Type: ");
    Serial.println(rfidData.creatureType);
    Serial.print("Creature Name: ");
    Serial.println(rfidData.creatureName);
}