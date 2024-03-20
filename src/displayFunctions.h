
#include <TFT_eSPI.h> // Include the TFT_eSPI library

#include "Trex.h"     // Include the T-Rex image
#include "treasure.h" // Include the treasure image

TFT_eSPI tft; // Create an instance of the TFT class

char quizAnswers[] = "000";

void clearScreen()
{
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
}

void lootBox()
{
  tft.fillScreen(TFT_BLACK); // Clear the screen
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.println("Return To\nTavern to\nopen loot\nbox");
  delay(3000);
  tft.fillScreen(TFT_BLACK); // Clear the screen
  tft.setSwapBytes(true);    // Swap the byte order for the display (if colors are wrong, change this value to false)
  tft.setCursor(0, 0);
  tft.pushImage(50, 0, 145, 128, treasure); // Draw the T-Rex image at the center of the screen
  delay(3000);
  return;
}

void displayTrex()
{
  tft.fillScreen(TFT_BLACK); // Clear the screen

  int16_t x = tft.width() / 2;                   // Calculate the x coordinate of the center of the screen
  int16_t y = tft.height() / 2;                  // Calculate the y coordinate of the center of the screen
  tft.pushImage(x - 30, y - 50, 128, 128, Trex); // Draw the T-Rex image at the center of the screen

  delay(3000);               // Wait for 3 seconds
  tft.fillScreen(TFT_BLACK); // Clear the screen
  return;
}

void scan4challange()
{
  tft.fillScreen(TFT_BLACK);

  // Draw a 3 pixel wide orange border around the screen
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_ORANGE);
  tft.drawRect(1, 1, tft.width() - 2, tft.height() - 2, TFT_ORANGE);
  tft.drawRect(2, 2, tft.width() - 4, tft.height() - 4, TFT_ORANGE);

  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.println("\n  Scan\n  challange\n  card.....");
}

void displayX()
{
  tft.fillScreen(TFT_BLACK); // Clear the screen

  tft.setTextDatum(MC_DATUM); // Set the datum to be the center of the screen
  tft.setTextSize(10);        // Set the text size to 5
  tft.setTextColor(TFT_RED);  // Set the text color to red

  int16_t x = tft.width() / 2;  // Calculate the x coordinate of the center of the screen
  int16_t y = tft.height() / 2; // Calculate the y coordinate of the center of the screen

  tft.drawString("X", x, y); // Draw the 'X' at the center of the screen

  delay(1000); // Wait for 3 seconds

  tft.fillScreen(TFT_BLACK); // Clear the screen
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
}

void displayCircle()
{
  tft.fillScreen(TFT_BLACK); // Clear the screen

  int16_t x = tft.width() / 2;  // Calculate the x coordinate of the center of the screen
  int16_t y = tft.height() / 2; // Calculate the y coordinate of the center of the screen
  int16_t radius = 50 / 2;      // Calculate the radius of the circle

  tft.fillCircle(x, y, radius, TFT_GREEN); // Draw the green circle

  delay(1000); // Wait for 3 seconds

  tft.fillScreen(TFT_BLACK); // Clear the screen
}

void animateEyes()
{
  int16_t x = tft.width() / 2;       // Calculate the x coordinate of the center of the screen
  int16_t y = tft.height() / 2 + 40; // Calculate the y coordinate of the center of the screen
  int16_t radius = 50 / 2;           // Calculate the radius of the circle
  int16_t pupilRadius = radius / 2;  // Calculate the radius of the pupil

  // Draw the white circles
  tft.fillCircle(x - radius - 10, y, radius, TFT_WHITE);
  tft.fillCircle(x + radius + 10, y, radius, TFT_WHITE);

  for (int i = -radius + pupilRadius + 2; i <= radius - pupilRadius - 2; i += 5)
  {
    // Clear the previous pupils
    tft.fillCircle(x - radius - 10 + i - 5, y, pupilRadius, TFT_WHITE);
    tft.fillCircle(x + radius + 10 + i - 5, y, pupilRadius, TFT_WHITE);

    // Draw the new pupils
    tft.fillCircle(x - radius - 10 + i, y, pupilRadius, TFT_BLACK);
    tft.fillCircle(x + radius + 10 + i, y, pupilRadius, TFT_BLACK);

    delay(200); // Wait for a short time
  }
}

void drawVerticalLine()
{
  int displayWidth = tft.width();
  int displayHeight = tft.height();

  int linePosition = (2 * displayWidth) / 3; // Calculate 2/3 of the display width

  tft.drawLine(linePosition, 0, linePosition, displayHeight, TFT_WHITE); // Draw a white line
}

void buttonReadText()
{
  drawVerticalLine();
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.println("\n               yes->\n\n\n\n               no->");
  tft.setTextSize(3);
}

void displayErrorMessage(const std::string &message)
{
  tft.fillScreen(TFT_BLACK); // Clear the screen

  tft.setTextSize(3);
  tft.setTextColor(TFT_RED);
  tft.setCursor(0, 0);
  tft.println(("\nError:\n" + message).c_str());

  delay(1000); // Wait for 3 seconds
  return;
}

void drawLineAcrossDisplay()
{
  int linePosition = tft.height() / 4;                                 // Calculate the position of the line
  tft.drawLine(0, linePosition, tft.width(), linePosition, TFT_WHITE); // Draw the line
}
