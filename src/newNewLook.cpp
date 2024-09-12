#include <Arduino.h>
#include <TFT_eSPI.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "mainFunctions.h"
#include "menuFunctions.h"
#include <SPI.h>

#include "Trex.h"    // Ensure this file contains the icon data
#include "objects.h" // Ensure this file contains the MenuOption class
#include "images/battle.h"
#include "images/coin.h" // Ensure this file contains the icon data
#include "images/player1.h"
#include "images/player2.h"
#include "images/player3.h"
#include "images/player4.h"
#include "images/fossil.h"
#include "images/noTrex.h"

void displayKey();
std::vector<std::string> addPlayer(int numPlayers);
void connectToNetwork();
std::pair<std::string, std::string> extractWordAndNumberString(const std::string &str);
String whatAnimal(std::vector<Player> &players);
void trialFunction();
std::vector<Player> players; // Declare the 'players' variable
void trialFunctionPOST(std::vector<Player> &players);
void addCreaturesPOST(std::vector<Player> &players);
void assignRandomValue(std::vector<Player> &players);
void extractWifiDetails();
void drawMenu();
HardwareSerial mySerial(1);

std::vector<Player> scanKey();

const int rotateButtonPin = 35; // Button1 on the TTGO T-Display
const int selectButtonPin = 0;  // Button2 on the TTGO T-Display

void function1();
void function2();
void function3();
void function4();
void function5();
void function6();
void function7();
void function8(String creatureCaptured);
void function9();

MenuOption options1[] = {
    MenuOption("1 PLAYER", player1, function1),
    MenuOption("2 PLAYERS", player2, function2),
    MenuOption("3 PLAYERS", player3, function3),
    MenuOption("4 PLAYERS", player4, function4)};

MenuOption options2[] = {
    MenuOption("FIND FOSSIL", fossil, function5),
    MenuOption("EARN LOOT", coin, function6),
    MenuOption("BATTLE", battle, function7)};

MenuOption options3[] = {
    MenuOption("KEEP FOSSIL", trex, []()
               { function8(capturedCreature); }), // Use a lambda to pass the argument
    MenuOption("DON'T KEEP", noTrex, function9)};

MenuOption *currentOptions = options1; // Pointer to the current menu options
int numMenuOptions = sizeof(options1) / sizeof(options1[0]);
int menuSelection = 0;

void drawMenu()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);

    // Draw menu options
    for (int i = 0; i < numMenuOptions; i++)
    {
        currentOptions[i].display(tft, 10, 10 + i * 30, i == menuSelection); // Pass tft as the first argument
    }

    // Draw the image corresponding to the selected menu option
    const uint16_t *image = currentOptions[menuSelection].getImage();
    if (image != nullptr)
    {
        tft.pushImage(160, 10, 80, 80, const_cast<uint16_t *>(image)); // Adjust the coordinates and size as needed
    }
}

void setup()
{
    Serial.begin(115200);
    mySerial.begin(9600, SERIAL_8N1, 27, 26); // Initialize serial communication on pins 27 (RX) and 26 (TX)
    tft.init();                               // Initialize the TFT display
    clearScreen();

    tft.begin();
    tft.setRotation(3);
    tft.setTextSize(2);

    tft.setSwapBytes(true);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set default text color

    // Initialize button pins
    pinMode(rotateButtonPin, INPUT_PULLUP);
    pinMode(selectButtonPin, INPUT_PULLUP);

    // Display the first menu (options1)
    currentOptions = options1;
    numMenuOptions = sizeof(options1) / sizeof(options1[0]);
    menuSelection = 0;
    drawMenu();
}

void loop()
{
    // Check if rotate button is pressed
    if (digitalRead(rotateButtonPin) == LOW)
    {
        menuSelection++;
        if (menuSelection >= numMenuOptions)
        {
            menuSelection = 0;
        }
        drawMenu();
        delay(200); // Debounce delay
    }

    // Check if select button is pressed
    if (digitalRead(selectButtonPin) == LOW)
    {
        if (currentOptions == options1)
        {
            switch (menuSelection)
            {
            case 0:
                function1();
                break;
            case 1:
                function2();
                break;
            case 2:
                function3();
                break;
            case 3:
                function4();
                break;
            default:
                break;
            }

            // Switch to options2 after any function is executed
            currentOptions = options2;
            numMenuOptions = sizeof(options2) / sizeof(options2[0]);
            menuSelection = 0; // Reset menu selection
            drawMenu();        // Redraw the menu
        }
        else if (currentOptions == options2)
        {
            switch (menuSelection)
            {
            case 0:
                function5();
                // Switch to options3 after function5 is executed
                currentOptions = options3;
                numMenuOptions = sizeof(options3) / sizeof(options3[0]);
                menuSelection = 0; // Reset menu selection
                drawMenu();        // Redraw the menu
                break;
            case 1:
                function6();
                break;
            case 2:
                function7();
                break;
            default:
                break;
            }
        }
        else if (currentOptions == options3)
        {
            if (menuSelection == 0)
            {
                function8(capturedCreature); // KEEP CREATURE
            }
            else if (menuSelection == 1)
            {
                function9(); // DON'T KEEP
            }

            // Return to options2 after handling options3
            currentOptions = options2;
            numMenuOptions = sizeof(options2) / sizeof(options2[0]);
            menuSelection = 0; // Reset menu selection
            drawMenu();        // Redraw the menu
        }

        delay(500); // Debounce delay
    }
}