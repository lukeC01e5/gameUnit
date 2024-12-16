// filepath: /c:/Users/OEM/Documents/GitHub/baseStation/src/RFID.h
#ifndef RFID_H
#define RFID_H

#include <MFRC522.h>

// Define RFID pins (ensure these are consistent across your project)
#define SS_PIN_RFID 13
#define RST_PIN_RFID 22
#define SCK_PIN 25
#define MISO_PIN 33
#define MOSI_PIN 26

// Declare external variables
extern MFRC522 mfrc522;
extern MFRC522::MIFARE_Key key;

// Function declarations
void initializeRFID();

#endif // RFID_H