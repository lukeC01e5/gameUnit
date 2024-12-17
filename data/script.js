// script.js

// Creature list
const creatures = [
    "None", "Flamingo", "Flame-Kingo", "Kitten", "Flame-on", "Pup", "Dog", "Wolf",
    "Birdy", "Haast-eagle", "Squidy", "Giant-Squid", "Kraken", "BabyShark", "Shark",
    "Megalodon", "Tadpole", "Poison-dart-frog", "Unicorn", "Master-unicorn", "Sprouty",
    "Tree-Folk", "Bush-Monster", "Baby-Dragon", "Dragon", "Dino-Egg", "T-Rex", "Baby-Ray",
    "Mega-Manta", "Orca", "Big-Bitey", "Flame-Lily", "Monster-Lily", "Bear-Cub", "Moss-Bear"
];

// Populate the creatureType select element
const creatureSelect = document.getElementById('creatureType');

for(let i = 1; i <= 34; i++) {
    const option = document.createElement('option');
    option.value = i;
    option.text = creatures[i];
    creatureSelect.add(option);
}