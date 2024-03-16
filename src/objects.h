#include <vector> // Add missing include statement
#include <string>

class Player {
public:
  std::string name;
  int healthLevel;
  std::string mainCreature;
  std::vector<std::string> altCreatures; // Add semicolon at the end
  std::vector<std::string> items;

  Player(std::string name, int healthLevel, std::string mainCreature)
    : name(name), healthLevel(healthLevel), mainCreature(mainCreature) {}
};
