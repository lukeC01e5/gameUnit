#include <Arduino.h>
#include <ostream>
#include <TFT_eSPI.h>       // Include the TFT library
#include <SPI.h>            // Include the SPI library
#include <HardwareSerial.h> // Include the HardwareSerial library
#include <string>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "arduino_secrets.h" // Include the file with the WiFi credentials
#include "displayFunctions.h"
#include "objects.h"
#include "buttonCode.h"

#include <WiFiClientSecure.h> // Include the WiFiClientSecure library

WiFiClientSecure client;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

bool hasPlayerBeenAdded = false; // Global variable
Player addAnotherPlayer();
void announcePlayerAdded(const std::string &playerName);

HardwareSerial mySerial(1); // Use the second hardware serial port

// const char *serverName = "https://gameapi-2e9bb6e38339.herokuapp.com";

const char *server = "https://gameapi-2e9bb6e38339.herokuapp.com";

void whatAnimal();
void postPlayerData(String data);
void getPlayerData();
void connectToNetwork();
void addPlayer();

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

  // client.setInsecure(); // Use this to disable certificate checking
}

void loop()
{

  if (!hasPlayerBeenAdded) // If a player has not been added
  {
    addPlayer(); // Call the addPlayer() function
  }

  if (mySerial.available()) // If there is data available to read
  {
    scan4challange();
    delay(1000); // Wait for 2 seconds
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

        // Copy std::string to char array
        strncpy(quizAnswers, quizA.c_str(), sizeof(quizAnswers));
        quizAnswers[sizeof(quizAnswers) - 1] = 0; // Ensure null-termination

        clearScreen();
        drawLineAcrossDisplay();
        writeQuestionsAtTop();
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
      connectToNetwork();
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

  getPlayerData();

  String data = "{\
\"firstName\": \"bob\",\
\"lastName\": \"dylan\",\
\"isAlive\": true,\
\"age\": 27,\
\"children\": [\
\"Catherine\",\
\"Thomas\",\
\"Trevor\"\
],\
\"spouse\": null\
}";

  // postPlayerData(data);
  delay(2000); // Wait for 10 seconds before next request
}

void getPlayerData()
{

  client.setInsecure(); // Use this to disable certificate checking (dodgy);
  tft.println("Connected to ");
  tft.println(ssid);

  clearScreen();

  tft.println("\nStarting connection to server...");
  client.setInsecure(); // skip verification
  if (!client.connect(server, 443))
  {
    clearScreen();
    tft.println("Connection failed!");
  }
  else
  {
    clearScreen();
    tft.println("Connected to server!");
    // Make a HTTP request:
    client.println("GET /api/v1/resources HTTP/1.1");
    // client.println("Host: gameapi-2e9bb6e38339.herokuapp.com");

    client.println("Host: gameapi-2e9bb6e38339.herokuapp.com");
    client.println("Connection: close");
    client.println();
  }

  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      Serial.println("headers received");
      tft.println("headers received");

      break;
    }
  }
  // if there are incoming bytes available
  // from the server, read them and print them:
  String response = ""; // create a String to store the response
  while (client.available())
  {
    char c = client.read();
    Serial.write(c);
    response += c; // append each character to the response string
  }
  clearScreen();
  tft.setTextSize(2);
  tft.println(response); // print the entire response
  Serial.println(response);

  delay(3000); // Wait for 2 seconds

  client.stop();
}

void postPlayerData(String data)
{
  client.setInsecure(); // Use this to disable certificate checking (dodgy);
  tft.println("Connected to ");
  tft.println(ssid);

  clearScreen();

  tft.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
  {
    clearScreen();
    tft.println("Connection failed!");
  }
  else
  {
    clearScreen();
    tft.println("Connected to server!");
    // Make a HTTP request:
    client.println("POST /api/v1/resources HTTP/1.1");
    client.println("Host: gameapi-2e9bb6e38339.herokuapp.com");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);
  }

  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      Serial.println("headers received");
      tft.println("headers received");
      break;
    }
  }

  String response = ""; // create a String to store the response
  while (client.available())
  {
    char c = client.read();
    Serial.write(c);
    response += c; // append each character to the response string
  }
  clearScreen();
  tft.setTextSize(2);
  tft.println(response); // print the entire response
  Serial.println(response);

  delay(3000); // Wait for 3 seconds

  client.stop();
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

  Player player(playerName, 100, "noCreature"); // Default healthLevel and mainCreature
  return player;
}

void assignItemToPlayer(Player &player, std::string item)
{
  player.items.push_back(item);
}

void addPlayer()
{
  if (hasPlayerBeenAdded)
    return; // If a player has already been added, return immediately

  clearScreen();
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, tft.getCursorY() + 10); // Move the cursor down
  tft.println("First Player\nName?");
  delay(1000); // Wait for 1 second

  Player player1 = createPlayerFromSerial(mySerial);
  if (player1.name.empty())
    return; // If player1's name is empty, return immediately

  announcePlayerAdded(player1.name);
  hasPlayerBeenAdded = true; // Set the global variable to true after a player has been added

  int confirmResult = buttonConfirm();
  if (confirmResult == 1)
  {
    Player player2 = addAnotherPlayer();
    if (!player2.name.empty())
      announcePlayerAdded(player2.name);

    confirmResult = buttonConfirm();
    if (confirmResult == 1)
    {
      Player player3 = addAnotherPlayer();
      if (!player3.name.empty())
      {
        announcePlayerAdded(player3.name);
        scan4challange();
        return;
      }
    }
    else if (confirmResult == 0)
    {
      scan4challange();
      return;
    }
  }
  else if (confirmResult == 0)
  {
    scan4challange();
    return;
  }
}

void announcePlayerAdded(const std::string &playerName)
{
  clearScreen();
  tft.println((playerName + " Added to\nteam!").c_str());
  delay(1500); // Wait for 1.5 seconds
  clearScreen();
  tft.println("Add\nanother\nplayer?");
  buttonReadText();
}

Player addAnotherPlayer()
{
  clearScreen();
  tft.println("Next Player\nName?");
  delay(1000); // Wait for 1 second
  return createPlayerFromSerial(mySerial);
}