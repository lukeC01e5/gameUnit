#include <vector> // Add missing include statement
#include <string>


#include <sstream>

class Player {
public:
  std::string name;
  std::string mainCreature;
  std::vector<std::string> items;

  Player(const std::string &name, const std::string &mainCreature, const std::vector<std::string> &items)
      : name(name), mainCreature(mainCreature), items(items) {}

  std::string toString() const {
    std::ostringstream oss;
    oss << "Name: " << name;
    /*
     << ", No creature: " << mainCreature;
    oss << ", Items: ";
    */  
    for (const auto& item : items) {
      oss << item << " ";
    }
    return oss.str();
  }
};


