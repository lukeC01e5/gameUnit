
/*
current problmes:
trialFunctionPOST() is not posting data, but all the playeres names are blanks
need to figiure out if its more important be multi player or single player,
Multiplayer is going to create real problems with the current code
still tring to figure out how close conection to client properly

*/
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" // Include the file with the WiFi credentials

#include "arduino_secrets.h" // Include the file with the WiFi credentials
#include "mainFunctions.h"

void displayKey();

std::vector<std::string> addPlayer();

void connectToNetwork();
std::pair<std::string, std::string> extractWordAndNumberString(const std::string &str);
String whatAnimal();
void trialFunction();
// void trialFunctionPOST(String playerNames[]);
// void trialFunctionPOST(std::vector<std::string> &playerNames);
std::vector<Player> players; // Declare the 'players' variable
void trialFunctionPOST(std::vector<Player> &players);
// void trialFunctionPOST(std::vector<Player> &players);
void assignRandomValue(std::vector<Player> &players);

std::vector<Player> scanKey();

TFT_eSPI tft = TFT_eSPI(); // Invoke the TFT_eSPI constructor to create the TFT object

// Declare quizAnswers variable

void setup()
{
    Serial.begin(115200);
    mySerial.begin(9600, SERIAL_8N1, 27, 26); // Initialize serial communication on pins 27 (RX) and 26 (TX)
    tft.init();                               // Initialize the TFT display
    clearScreen();
    tft.setRotation(1);
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE);

    // Initialize the button pins as input
    pinMode(yesButtonPin, INPUT_PULLUP);
    pinMode(noButtonPin, INPUT_PULLUP);

    String word = "";
}

void loop()
{
    if (!hasPlayerBeenAdded)
    {
        addPlayer();
    }

    if (mySerial.available()) // If there is data available to read
    {
        scan4challange();
        delay(500); // Wait for 2 seconds
        String barcode = mySerial.readString();
        if (!barcode.isEmpty())
        {
            std::string extractWord(const std::string &str);
            tft.setCursor(0, 0);
            std::string myText = barcode.c_str(); // Convert Arduino String to std::string
            std::pair<std::string, std::string> result = extractWordAndNumberString(myText);
            std::string word = result.first;
            std::string quizA = result.second;

            if (word == "challenge")
            {
                displayCircle();
                tft.setCursor(0, 0);
                std::string message = "Challenge \ncard loaded " + quizA;
                tft.println(message.c_str());
                //
                // Copy std::string to char array
                strncpy(quizAnswers, quizA.c_str(), sizeof(quizAnswers));
                quizAnswers[sizeof(quizAnswers) - 1] = 0; // Ensure null-termination

                clearScreen();
                drawLineAcrossDisplay();
                writeQuestionsAtTop();
                delay(2000); // Wait for 2 seconds
                // lootBox();
                connectToNetwork();
                std::vector<Player> players = scanKey();
                clearScreen();
                assignRandomValue(players);

                trialFunctionPOST(players); // Pass the 'players' variable to the function
            }
            else if (word == "creatureCapture")
            {
                clearScreen();
                tft.println("Seek Creature\n to be your");
                whatAnimal();
            }
            else
            {
                displayX();
                const std::string message = "Invalid\nscan: " + word;
                displayErrorMessage(message);
                delay(1000); // Wait for 2 seconds
                scan4challange();
            }
        }
        else
        {
            Serial.println("Empty barcode");
            const std::string message = "Empty barcode: + barcode.c_str()";
            displayErrorMessage(message);
        }
    }
}