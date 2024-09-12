
#ifndef OBJECTS_H
#define OBJECTS_H

#include <Arduino.h>
#include <vector>
#include <string>
#include <sstream>
#include <TFT_eSPI.h>
#include <pgmspace.h>

TFT_eSPI tft = TFT_eSPI();

class Player
{
public:
  std::string name;
  std::string mainCreature;
  std::vector<std::string> items;
  std::vector<std::string> creatures; // Add this line

  Player(const std::string &name, const std::string &mainCreature, const std::vector<std::string> &items, const std::vector<std::string> &creatures)
      : name(name), mainCreature(mainCreature), items(items), creatures(creatures) {} // Modify this line

  std::string toString() const
  {
    std::ostringstream oss;
    oss << "Name: " << name;

    for (const auto &item : items)
    {
      oss << item << " ";
    }
  
    for (const auto &creature : creatures)
    {
      oss << creature << " ";
    }
    return oss.str();
  }
};

class MenuOption
{
public:
  MenuOption(const char *name, const uint16_t *image, void (*action)())
      : name(name), image(image), action(action) {}

  void display(TFT_eSPI &tft, int x, int y, bool selected) const
  {
    tft.setCursor(x, y);
    if (selected)
    {
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    }
    else
    {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.println(name);
  }

  void trigger() const
  {
    action();
  }

  const uint16_t *getImage() const
  {
    return image;
  }

private:
  const char *name;
  const uint16_t *image;
  void (*action)();
};

extern MenuOption options3[];

#endif // OBJECTS_H