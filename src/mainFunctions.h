#ifndef MAIN_FUNCTIONS_H
#define MAIN_FUNCTIONS_H

#include <Arduino.h>
#include <TFT_eSPI.h>       // Include the TFT library
#include <SPI.h>            // Include the SPI library
#include <HardwareSerial.h> // Include the HardwareSerial library
#include <WiFi.h>
#include <WiFiClient.h>
#include <sstream>
#include <ArduinoJson.h>
#include <random>
#include <vector>
#include <unordered_map>

#include "arduino_secrets.h" // Include the file with the WiFi credentials
#include "displayFunctions.h"
#include "objects.h"
#include "buttonCode.h"
// #include "treasureUnlock.h"

char quizAnswers[10];

extern TFT_eSPI tft;

bool hasPlayerBeenAdded = false; // Global variable

std::vector<std::string> playerNames;

String creatureCaptured = "noCreature";

HardwareSerial mySerial(1); // Use the second hardware serial port

WiFiClient client;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

std::string announcePlayerAdded(const std::string &playerName);
std::vector<Player> createPlayers(const std::vector<std::string> &playerNames);

Player addAnotherPlayer();

#endif

// std::string announcePlayerAdded(const std::string &playerName);

void wrongAnswer();
void assignRandomValue(std::vector<Player> &players);

std::pair<std::string, std::string> extractWordAndNumberString(const std::string &str)
{
    std::string startDelimiter = "11--";
    std::string endDelimiter = "--11";

    // Find the start and end positions of the word
    size_t startPos = str.find(startDelimiter);
    size_t endPos = str.find(endDelimiter);

    // If the start or end delimiter was not found, return two empty strings
    if (startPos == std::string::npos || endPos == std::string::npos)
    {
        return {"", ""};
    }

    // Adjust the start position to be after the start delimiter
    startPos += startDelimiter.length();

    // Calculate the length of the word
    size_t length = endPos - startPos;

    // Extract the word
    std::string wordAndNumber = str.substr(startPos, length);

    // Split the word and number at the full stop
    size_t dotPos = wordAndNumber.find('.');
    if (dotPos == std::string::npos)
    {
        return {"", ""}; // Return two empty strings if the full stop was not found
    }

    std::string word = wordAndNumber.substr(0, dotPos);
    std::string quizA = wordAndNumber.substr(dotPos + 1);

    return {word, quizA};
}

std::unordered_map<std::string, std::function<void()>> animalHandlers = {
    {"babyDragon", []() { /* handle babyDragon */ }},
    {"dinoEgg", []() { /* handle dinoEgg */ }},
    {"wolfPup", []() { /* handle wolfPup */ }},
    {"kitten", []() { /* handle kitten */ }},
    {"chicky", []() { /* handle chicky */ }},
    {"fishy", []() { /* handle fishy */ }},
    {"squidy", []() { /* handle squidy */ }},
    {"larve", []() { /* handle larve */ }},
    {"sprouty", []() { /* handle sprouty */ }},
    {"roboCrab", []() { /* handle roboCrab */ }},
    {"ghost", []() { /* handle ghost */ }},
    {"babyTrex", []()
     {
         displayTrex();
         tft.setCursor(0, 0);
         tft.println("Keep\nCreature\nas\nyour\nchampion");
         delay(2000); // Wait for 2 seconds
         buttonReadText();
         buttonConfirm();
     }}};

String whatAnimal(Player &player)
{
    tft.println(" champion");
    delay(1000); // Wait for 1 seconds

    String input = ""; // Declare the variable "input"

    unsigned long startMillis = millis();
    unsigned long currentMillis = startMillis;

    while (!mySerial.available() && (currentMillis - startMillis <= 20000)) // 5 seconds timeout
    {
        // wait for data to be available
        delay(100); // optional delay to prevent the loop from running too fast
        animateEyes();
        currentMillis = millis();
    }

    if (mySerial.available())
    {
        // now read from Serial
        input = mySerial.readString();
    }
    else
    {
        tft.println("No serial data received");
        return "";
    }

    String word = "";
    clearScreen();

    std::string myText = input.c_str(); // Convert Arduino String to std::string

    std::pair<std::string, std::string> result = extractWordAndNumberString(myText);

    std::string animal = result.first;
    std::string number = result.second;

    if (animalHandlers.count(animal) > 0)
    {
        // Call the corresponding handler
        animalHandlers[animal]();

        // Update the player's mainCreature
        player.mainCreature = animal;

        // Instruct the player to return to base
        clearScreen();
        tft.println("creature has been captured do you want to keep the creature`");
        delay(1000); // Wait for 1 second

        clearScreen();

        if (buttonConfirm() == 1)
        {
            clearScreen();
            tft.println("Return to\nbase to\nkeep");
            creatureCaptured = animal.c_str();
            delay(1000); // Wait for 2 seconds
            return String(creatureCaptured);
        }
        else if (buttonConfirm() == 0)
        {
            clearScreen();
            tft.println("\nPick another\ncreature");
            delay(1000); // Wait for 2 seconds
            return "";
        }
    }
    else
    {
        tft.println("Invalid input: " + input);
        // scan4challange();
        return "";
    }

    return "";
}

void connectToNetwork()
{
    WiFi.begin(ssid, pass); // Connect to the network
    delay(1000);            // Wait for 1 second

    while (WiFi.status() != WL_CONNECTED)
    {
        clearScreen();
        tft.println("Connecting to WiFi...");
        // delay(1000); // Wait for 1 second
    }
    clearScreen();
    tft.println("Connected to WiFi!");
    delay(500); // Wait for 1 second
}

Player createPlayerFromSerial(HardwareSerial &mySerial)
{
    std::string playerName;
    int playerNumber = -1;
    unsigned long startTime = millis();
    unsigned long timeout = 10000; // 5 seconds

    while (millis() - startTime < timeout)
    {
        if (mySerial.available())
        {
            playerName = mySerial.readStringUntil('\n').c_str(); // Read until newline
            std::pair<std::string, std::string> result = extractWordAndNumberString(playerName);

            playerName = result.first;               // Update playerName with the extracted word
            playerNumber = std::stoi(result.second); // Convert the extracted number string to an integer using std::stoi and update playerNumber

            if (playerNumber == 999)
            {
                break;
            }
            else
            {
                displayX();
            }
        }
    }

    Player player(playerName, "noCreature", std::vector<std::string>());
    return player;
}

std::vector<std::string> addPlayer()
{
    if (hasPlayerBeenAdded)
        return playerNames; // If a player has already been added, return immediately

    clearScreen();
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(3);
    tft.setCursor(0, tft.getCursorY() + 10); // Move the cursor down
    tft.println("\nFirst Player\n    Name?");
    delay(1000); // Wait for 1 second

    Player player1 = createPlayerFromSerial(mySerial);
    if (!player1.name.empty())
    {
        std::string player1Name = announcePlayerAdded(player1.name);
        playerNames.push_back(player1Name); // Add player1's name to the vector

        Serial.println(("Added player: " + player1Name).c_str()); // Print the player name
        hasPlayerBeenAdded = true;                                // Set hasPlayerBeenAdded to true after first player is added
    }

    for (int i = 0; i < 3; i++) // Loop for the next 3 players
    {
        int confirmResult = buttonConfirm();
        if (confirmResult == 1)
        {
            Player nextPlayer = addAnotherPlayer();
            if (!nextPlayer.name.empty())
            {
                std::string nextPlayerName = announcePlayerAdded(nextPlayer.name);
                playerNames.push_back(nextPlayerName);

                Serial.println(("Added player: " + nextPlayerName).c_str());
            }
        }
        else if (confirmResult == 0)
        {
            clearScreen();
            tft.println("\n Team\nComplete!");
            delay(2000); // Wait for 1 second
            clearScreen();
            return playerNames;
        }
    }

    clearScreen();
    return playerNames;
}

std::string announcePlayerAdded(const std::string &playerName)
{
    clearScreen();
    tft.println((playerName + "\nAdded to\nteam!").c_str());
    delay(1500); // Wait for 1.5 seconds
    clearScreen();
    tft.setTextWrap(true);
    tft.println("\nAdd\nanother\nplayer?");
    buttonReadText();

    return playerName;
}

Player addAnotherPlayer()
{
    clearScreen();
    tft.println("Next Player\nName?");
    delay(1000); // Wait for 1 second
    return createPlayerFromSerial(mySerial);
}

void writeQuestionsAtTop()
{
    tft.setCursor(0, 3);
    tft.setTextColor(TFT_WHITE); // Set the text color to white        // Set the cursor to the top left corner
    tft.setTextSize(3);          // Set the text size
    tft.println(" Questions");   // Print the text
    tft.setTextSize(2);

    for (int i = 0; i < 3; ++i)
    {
        // Display "Question X: "
        tft.print("\n Question ");
        tft.print(i + 1);
        tft.print(":");

        // Initially display a "?"
        tft.println("?");

        int y = tft.getCursorY(); // Save the current cursor y position

        while (true) // Start of the while loop
        {
            // Wait for a value from 'mySerial'
            while (!mySerial.available())
            {
                delay(100); // Wait for 100 milliseconds
            }

            // Read the value from 'mySerial'
            String value = mySerial.readString();

            // Trim any leading or trailing whitespace from 'value'
            value.trim();

            // If the value equals 'quizAnswers[i]', display "correct!"
            if (value == String(quizAnswers[i]))
            {
                tft.fillRect(tft.getCursorX() + 140, y - tft.fontHeight(), tft.textWidth("?") + 5, tft.fontHeight(), TFT_BLACK); // Draw a black rectangle over the "?"
                tft.setCursor(tft.getCursorX() + 140, y - tft.fontHeight());
                tft.setTextColor(TFT_GREEN); // Set the cursor to the start of the line
                tft.println("Correct!");     // Print "Correct!"
                tft.setTextColor(TFT_WHITE); // Set the text color to white
                break;                       // Break out of the while loop
            }
            else if (value != String(quizAnswers[i]))
            {
                wrongAnswer();
                wrongAnswer();
                wrongAnswer();
            }
        }
    }
}

void trialFunctionPOST(std::vector<Player> &players)
{
    tft.setTextSize(2);

    displayCircleOrange();

    tft.println("part0");
    delay(1000);

    if (players.empty())
    {
        tft.println("No players");
        return;
    }

    for (Player &player : players)
    {
        clearScreen();

        // Connect to the server
        if (client.connect("gameapi-2e9bb6e38339.herokuapp.com", 80))
        {
            for (const std::string &item : player.items)
            {
                // Send the POST request
                client.println(("POST /api/v1/users/" + player.name + "/add_" + item + " HTTP/1.1").c_str());
                client.println("Host: gameapi-2e9bb6e38339.herokuapp.com");
                client.println("Content-Type: application/json");
                client.println("Connection: close");
                client.println("User-Agent: Arduino/1.0");
                client.println("Content-Length: 0");
                client.println();

                // Wait for the response
                unsigned long timeout = millis();
                while (client.connected() || client.available())
                {
                    if (millis() - timeout > 5000)
                        break;

                    if (client.available())
                    {
                        String line = client.readStringUntil('\n');
                        if (line == "\r")
                        {
                            // Headers received. Print the actual data
                            tft.fillScreen(TFT_BLACK); // Clear the screen
                            tft.setCursor(0, 0);
                            while (client.available())
                            {
                                String line = client.readStringUntil('\n');
                                tft.println(line);
                            }
                            break;
                        }
                        else
                        {
                            // Print the headers
                            tft.println(line);
                        }
                    }
                }

                // Close the connection
                client.stop();
                delay(1000);
            }
        }
        else
        {
            tft.println("Connection failed");
        }
    }
}

std::vector<Player> scanKey()
{
    bool validInput = false;
    std::vector<Player> players;

    while (!validInput)
    {
        tft.println("Find key\nto open\nloot box");
        delay(2000); // Wait for 1 second
        clearScreen();
        displayKey();
        delay(2000);       // Wait for 1 second
        String input = ""; // Declare the variable "input"
        clearScreen();

        while (!mySerial.available())
        {
            // wait for data to be available
            delay(100); // optional delay to prevent the loop from running too fast
            animateEyes();
        }

        // now read from Serial
        input = mySerial.readString();
        clearScreen();

        std::string myText = input.c_str(); // Convert Arduino String to std::string

        std::pair<std::string, std::string> result = extractWordAndNumberString(myText);

        std::string key = result.first;
        std::string number = result.second;

        if (key == "key")
        {
            displayKey();
            tft.setCursor(0, 0);
            tft.println("Key scanned");
            delay(2000); // Wait for 2 seconds
            displayKey();
            clearScreen();
            validInput = true;

            // players = createPlayers(playerNames);
            displayCircle();

            for (const auto &player : players)
            {
                tft.println(player.toString().c_str());
            }
            delay(2000);
            return players;
        }
        else
        {
            tft.println("Invalid input: " + input);
        }
    }
    return players;
}

// Create a random number generator
std::random_device rd;
std::mt19937 gen(rd());

void assignRandomValue(std::vector<Player> &players)
{
    // Values vector
    static const std::vector<std::string> values = {"crystal", "plant", "meat", "water", "coin"};
    std::uniform_int_distribution<> dis(0, values.size() - 1);

    displayCircleOrange();

    // Assign a random value to each player
    for (Player &player : players)
    {
        clearScreen();

        int randomIndex = dis(gen);
        tft.println("item added:\n " + String(randomIndex));
        delay(2000);
        player.items.push_back(values[randomIndex]);

        // Print the player's information
        std::string playerStr = player.toString();
        tft.println(playerStr.c_str());
    }
}

std::vector<Player> createPlayers(const std::vector<std::string> &playerNames)
{
    std::vector<Player> players;
    for (const auto &name : playerNames)
    {
        Player player(name, "noCreature", std::vector<std::string>());
        players.push_back(player);
    }
    return players;
}