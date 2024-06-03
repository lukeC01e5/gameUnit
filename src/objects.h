#include <vector>
#include <string>
#include <sstream>

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
    // Add this block to print creatures
    // oss << ", Creatures:";
    for (const auto &creature : creatures)
    {
      oss << creature << " ";
    }
    return oss.str();
  }
};
