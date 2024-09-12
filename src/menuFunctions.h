
#ifndef MENUFUNCTIONS_H
#define MENUFUNCTIONS_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "displayFunctions.h"
#include "mainFunctions.h"
#include "menuFunctions.h"
#include <SPI.h>
#include "objects.h"       // Ensure this file contains the MenuOption class
#include "images/fossil.h" // Include another image data
#include "images/lab.h"

extern std::vector<Player> players; // Declare the players variable
String capturedCreature = "";

void function1()
{
    if (!hasPlayerBeenAdded)
    {
        int numPlayersToAdd = 1; // Example: Add 4 players
        playerNames = addPlayer(numPlayersToAdd);
        players = createPlayers(playerNames);

        for (const auto &player : players)
        {
            std::string playerStr = player.toString();
            tft.println(playerStr.c_str());
            delay(2000);
        }
    }
}

void function2()
{
    if (!hasPlayerBeenAdded)
    {
        int numPlayersToAdd = 2; // Example: Add 4 players
        playerNames = addPlayer(numPlayersToAdd);
        players = createPlayers(playerNames);

        for (const auto &player : players)
        {
            std::string playerStr = player.toString();
            tft.println(playerStr.c_str());
            delay(2000);
        }
    }
}

void function3()
{
    if (!hasPlayerBeenAdded)
    {
        int numPlayersToAdd = 3; // Example: Add 4 players
        playerNames = addPlayer(numPlayersToAdd);
        players = createPlayers(playerNames);

        for (const auto &player : players)
        {
            std::string playerStr = player.toString();
            tft.println(playerStr.c_str());
            delay(2000);
        }
    }
}

void function4()
{
    if (!hasPlayerBeenAdded)
    {
        int numPlayersToAdd = 4; // Example: Add 4 players
        playerNames = addPlayer(numPlayersToAdd);
        players = createPlayers(playerNames);

        for (const auto &player : players)
        {
            std::string playerStr = player.toString();
            tft.println(playerStr.c_str());
            delay(2000);
        }
    } // Code for function 3
}

void function5()
{
    // Wait for 0.5 seconds
    String input = ""; // Declare the variable "input"
    clearScreen();
    delay(500);

    // Wait for data to be available with a 5 seconds timeout
    while (!mySerial.available())
    {
        delay(100); // Optional delay to prevent the loop from running too fast
        scanAnimation(fossil);

        // animateEyes();
    }

    if (mySerial.available())
    {
        // Read from Serial
        input = mySerial.readString();
    }
    else
    {
        tft.println("No serial data received");
        return;
    }

    std::string myText = input.c_str(); // Convert Arduino String to std::string
    auto result = extractWordAndNumberString(myText);
    std::string animal = result.first;
    std::string number = result.second;

    if (animalHandlers.count(animal) > 0)
    {
        // Call the corresponding handler
        {
            // Call the corresponding handler
            animalHandlers[animal]();
            clearScreen();
            tft.setTextSize(2);
            tft.println("Creature has been captured");
            delay(2000);
            clearScreen();
            // tft.println("Return to\nbase to\nkeep");
            creatureCaptured = animal.c_str();

            // Convert creatureCaptured to std::string
            std::string capturedCreature = creatureCaptured.c_str();

            // Update all players
            updatePlayers(players, capturedCreature);

            // Remove the return statement to allow displayCircle() to be called
            displayCircle();
        }
    }
    else
    {
        tft.println("Invalid input: " + input);
    }
}

void function6()
{
    scan4challange();
    // Loop until data is available to read
    while (!mySerial.available())
    {
        delay(100); // Wait for 100 milliseconds before checking again
    }

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
            strncpy(quizAnswers, quizA.c_str(), sizeof(quizAnswers));
            quizAnswers[sizeof(quizAnswers) - 1] = 0; // Ensure null-termination
            clearScreen();
            drawLineAcrossDisplay();
            writeQuestionsAtTop();
            delay(2000); // Wait for 2 seconds
            clearScreen();
            scanKey();
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

void function7()
{
    clearScreen();
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("  NOT ENOUGH\n  POINTS FOR\n  BATTLE");
    delay(2000);
}

void function8(String creatureCaptured)
{
    Serial.println(" fossil scanned: " + creatureCaptured);
    if (!players.empty())
    {
        for (Player &player : players)
        {
            String capturedCreature = creatureCaptured.c_str();
        }
        clearScreen();
        tft.println(" Return to lab to\n upload DNA");
        scanAnimation(lab);
        extractWifiDetails();
        addCreaturesPOST(players);
    }
    else
    {
        clearScreen();
        tft.println("No players available");
    }
}

void function9()
{
}

#endif // MENUFUNCTIONS_H