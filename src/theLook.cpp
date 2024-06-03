
/*
current problmes:
trialFunctionPOST() is not posting data, but all the playeres names are blanks
need to figiure out if its more important be multi player or single player,
Multiplayer is going to create real problems with the current code
still tring to figure out how close conection to client properly


gets as far as Earn Loot or capture creature, then stops you can press a button but it will just repeat
itself

*/
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" // Include the file with the WiFi credentials
#include "mainFunctions.h"

void displayKey();

std::vector<std::string> addPlayer();

void connectToNetwork();
std::pair<std::string, std::string> extractWordAndNumberString(const std::string &str);
String whatAnimal(Player &player);
void trialFunction();
std::vector<Player> players; // Declare the 'players' variable
void trialFunctionPOST(std::vector<Player> &players);
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
        playerNames = addPlayer();
        players = createPlayers(playerNames);

        for (const auto &player : players)
        {
            std::string playerStr = player.toString();
            tft.println(playerStr.c_str());
            delay(2000);
        }
    }
    clearScreen();
    tft.setTextSize(3);
    tft.println("Earn loot ->\n\n\nCapture\ncreature  ->");

    int buttonValue = buttonConfirm();
    while (buttonValue != 0 && buttonValue != 1)
    {
        delay(100); // Wait for 100 milliseconds
        buttonValue = buttonConfirm();
    }
    clearScreen();

    if (buttonValue == 1)
    {
        delay(500); // Wait for 2 seconds
        scan4challange();
        // Loop until data is available to read
        while (!mySerial.available())
        {
            delay(100); // Wait for 100 milliseconds before checking again
        }

        // delay(500); // Wait for 2 seconds

        // ... (the rest of your function remains the same)

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
                scanKey();
                connectToNetwork();
                clearScreen();
                assignRandomValue(players);

                trialFunctionPOST(players); // Pass the 'players' variable to the function
            }
        }
        else
        {
            Serial.println("Empty barcode");
            const std::string message = "Empty barcode: + barcode.c_str()";
            displayErrorMessage(message);
        }
    }
    else if (buttonValue == 0)
    {
        clearScreen();
        std::vector<std::string> playerNames = addPlayer();
        //  std::vector<Player> scanKey();
        // players = scanKey();
        tft.println("Seek Creature\n to be your");
        if (!players.empty())
        {
            for (Player &player : players)
            {
                whatAnimal(player);
                trialFunctionPOST(players);
            }
        }
        else
        {
            clearScreen();
            tft.println("No players available");
        }
    }
    else
    {
        displayErrorMessage("Invalid button value");
    }
}
