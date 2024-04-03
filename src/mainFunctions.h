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

#include "arduino_secrets.h" // Include the file with the WiFi credentials
#include "displayFunctions.h"
#include "objects.h"
#include "buttonCode.h"
// #include "treasureUnlock.h"

char quizAnswers[10];

extern TFT_eSPI tft;

bool hasPlayerBeenAdded = false; // Global variable

std::vector<std::string> playerNames;
// String playerNames[] = {"", "", ""}; // Array to store the player names

HardwareSerial mySerial(1); // Use the second hardware serial port

WiFiClient client;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

#endif

// void announcePlayerAdded(const std::string &playerName, std::vector<std::string> &playerNames);
std::string announcePlayerAdded(const std::string &playerName);
Player addAnotherPlayer();

void wrongAnswer();

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

void whatAnimal()
{
    tft.println(" champion");
    delay(1000); // Wait for 1 seconds

    String input = ""; // Declare the variable "input"

    while (!mySerial.available())
    {
        // wait for data to be available
        delay(100); // optional delay to prevent the loop from running too fast
        animateEyes();
    }

    // now read from Serial
    input = mySerial.readString();

    String word = "";
    clearScreen();

    std::string myText = input.c_str(); // Convert Arduino String to std::string

    std::pair<std::string, std::string> result = extractWordAndNumberString(myText);

    std::string animal = result.first;
    std::string number = result.second;

    if (animal == "babyTrex")
    {
        displayTrex();
        tft.setCursor(0, 0);
        tft.println("Keep\nT-rex\nas\nyour\nchampion");
        delay(2000); // Wait for 2 seconds
        buttonReadText();
        buttonConfirm();

        if (buttonConfirm() == 1)
        {
            clearScreen();
            tft.println("Return to\ntavern to\nkeep\ncreature");
            delay(1000); // Wait for 2 seconds
                         // connectToNetwork();
        }
        else if (buttonConfirm() == 0)
        {
            clearScreen();
            tft.println("\nPick another\ncreature");
            delay(1000); // Wait for 2 seconds
            return;
        }
    }

    else
    {
        tft.println("Invalid input: " + input);
        scan4challange();
        return;
    }
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
    // int playerNumber;

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

    // Player player(playerName, 100, "noCreature"); // Need to unedit this to return it to normal
    Player player(playerName, 100, "noCreature");
    return player;
}

std::vector<std::string> addPlayer()
{
    // std::vector<std::string> playerNames;

    if (hasPlayerBeenAdded)
        return playerNames; // If a player has already been added, return immediately

    clearScreen();
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(3);
    tft.setCursor(0, tft.getCursorY() + 10); // Move the cursor down
    tft.println("First Player\nName?");
    delay(1000); // Wait for 1 second

    Player player1 = createPlayerFromSerial(mySerial);
    if (player1.name.empty())
        return playerNames; // If player1's name is empty, return immediately

    std::string player1Name = announcePlayerAdded(player1.name);
    // playerNames.push_back(player1Name);

    std::string playerNamesString;
    for (const auto &name : playerNames)
    {
        playerNamesString += name + ",";
    }

    hasPlayerBeenAdded = true; // Set the global variable to true after a player has been added

    int confirmResult = buttonConfirm();
    if (confirmResult == 1)
    {
        playerNames.push_back(player1Name);
        Player player2 = addAnotherPlayer();
        if (!player2.name.empty())
        {
            std::string player2Name = announcePlayerAdded(player2.name);
            playerNames.push_back(player2Name);
        }

        confirmResult = buttonConfirm();
        if (confirmResult == 1)
        {
            Player player3 = addAnotherPlayer();
            if (!player3.name.empty())
            {
                std::string player3Name = announcePlayerAdded(player3.name);
                playerNames.push_back(player3Name);
                scan4challange();
            }
        }
        else if (confirmResult == 0)
        {
            scan4challange();
        }
    }
    else if (confirmResult == 0)
    {
        scan4challange();
    }

    std::stringstream ss(playerNamesString);
    std::string token;
    while (std::getline(ss, token, ','))
    {
        playerNames.push_back(token);
    }
    return playerNames;
}

std::string announcePlayerAdded(const std::string &playerName)
{
    clearScreen();
    tft.println((playerName + " Added to\nteam!").c_str());
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

void assignItemToPlayer(Player &player, std::string item)
{
    player.items.push_back(item);
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

void trialFunction()
{
    // Connect to the server
    if (client.connect("gameapi-2e9bb6e38339.herokuapp.com", 80))
    {
        // Send the GET request
        client.println("GET /api/v1/resources HTTP/1.1");
        client.println("Host: gameapi-2e9bb6e38339.herokuapp.com");
        client.println("Connection: close");
        client.println();

        // Wait for the response
        while (client.connected() || client.available())
        {
            if (client.available())
            {
                String line = client.readStringUntil('\n');
                if (line == "\r")
                {
                    // Headers received. Print the actual data
                    while (client.available())
                    {
                        tft.fillScreen(TFT_BLACK); // Clear the screen
                        tft.setCursor(0, 0);
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
    }
    else
    {
        tft.println("Connection failed");
    }

    // Close the connection
    client.stop();
    delay(10000);
}

// void trialFunctionPOST(String playerNames[])

// Change the argument type of trialFunctionPOST to std::vector<std::string>&
void trialFunctionPOST(std::vector<std::string> &playerNames)
{
    tft.setTextSize(2);
    // Create a JSON document
    // ArduinoJson::JsonDocument doc(200);
    // StaticJsonDocument doc(200);
    // StaticJsonDocument<200> doc;
    // Replace StaticJsonDocument with DynamicJsonDocument
    DynamicJsonDocument doc(200);

    // Fix the syntax error before the void keyword
    // This depends on the preceding code, which is not shown in the error message
    // JsonDocument doc(200);

    // Add data to the JSON document
    doc["firstName"] = playerNames[0];
    doc["secondName"] = playerNames[1];
    doc["thirdName"] = playerNames[2];
    // doc["creature"] = "T-rex";
    //  doc["Item"]

    // Convert the JSON document to a string
    String jsonData;
    serializeJson(doc, jsonData);

    // Connect to the server
    if (client.connect("gameapi-2e9bb6e38339.herokuapp.com", 80))
    {

        // Send the POST request
        client.println("POST /api/v1/resources HTTP/1.1");
        client.println("Host: gameapi-2e9bb6e38339.herokuapp.com");
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(jsonData.length());
        client.println();
        client.println(jsonData);

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
    }
    else
    {
        tft.println("Connection failed");
    }

    // Close the connection
    client.stop();
    delay(10000);
}
