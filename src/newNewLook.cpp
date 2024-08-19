#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "coin.h"    // Ensure this file contains the icon data
#include "Trex.h"    // Ensure this file contains the icon data
#include "objects.h" // Ensure this file contains the MenuOption class
#include "battle.h"
#include "mainFunctions.h"

// TFT_eSPI tft = TFT_eSPI();

const int rotateButtonPin = 35; // Button1 on the TTGO T-Display
const int selectButtonPin = 0;  // Button2 on the TTGO T-Display

void function1()
{
  clearScreen();
  tft.println("Find Creature\n fossil to\n");
  // Your function1 code here
}

void function2()
{
  // Code for function 2
}

void function3()
{
  // Code for function 3
}

MenuOption options[] = {
    MenuOption("FIND ANIMAL", trex, function1),
    MenuOption("EARN LOOT", coin, function2),
    MenuOption("BATTLE", battle, function3)};

int selectedOption = 0;

int menuSelection = 0;
const int numMenuOptions = sizeof(options) / sizeof(options[0]);

void drawMenu()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);

  // Draw menu options
  for (int i = 0; i < numMenuOptions; i++)
  {
    options[i].display(tft, 10, 10 + i * 30, i == menuSelection); // Pass tft as the first argument
  }

  // Draw the image corresponding to the selected menu option
  const uint16_t *image = options[menuSelection].getImage();
  if (image != nullptr)
  {
    tft.pushImage(160, 10, 80, 80, const_cast<uint16_t *>(image)); // Adjust the coordinates and size as needed
  }
}

void setup()
{
  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(2);

  tft.setSwapBytes(true);

  tft.fillRect(0, 0, 80, 80, TFT_RED);    // Draw a red rectangle
  tft.fillRect(80, 0, 80, 80, TFT_GREEN); // Draw a green rectangle
  tft.fillRect(160, 0, 80, 80, TFT_BLUE); // Draw a blue rectangle

  delay(2000); // Delay for 1 second

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set default text color
  drawMenu();

  // Initialize button pins
  pinMode(rotateButtonPin, INPUT_PULLUP);
  pinMode(selectButtonPin, INPUT_PULLUP);
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
    options[menuSelection].trigger();
    delay(500); // Debounce delay
  }
}