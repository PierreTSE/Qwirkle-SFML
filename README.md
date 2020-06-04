# Qwirkle

The [Qwirkle](https://en.wikipedia.org/wiki/Qwirkle) game in C++, based on the [SFML](https://www.sfml-dev.org/index.php).

![alt text](img/screenshot.jpg "Screenshot of the game")

### Installation from source

You can compile the source code using CMake and a C++17 compliant compiler.

### Download

A compiled version for [Windows](https://drive.google.com/file/d/1elfCsmwQ7toCZiiMhGNUwZyvWQyTgNfv/view?usp=sharing).
If you miss some dll you can get them from [here](https://drive.google.com/file/d/1NjZyAm01Dt6L9UDjyuKRKOkHVZMhavlX/view?usp=sharing).

### How to play

Select the number of players and computer using the arrow keys, and hit Start to begin playing. The computers are greedy and play the first of the moves that score the most.

###### Controls

| Action | Result |
| --- | --- |
| Drag and drop | Move grid |
| Mouse wheel | Zoom in / out |
| Left click | Select tile |
| Right click | Cancel
| C (center) | Center grid |
| H (hint)| Show playable squares for the currently selected tile |
| R (recycle), or click on the Recycling Logo | Select tiles to put back in reserve, same action to confirm |
| E (end), or click on the Right Arrow Logo | End turn |
| 1 - 6 keys | Select the corresponding tile |

### Configuration

You can change configuration parameters in the file ./rc/config.txt . \
You can add a locale by copying and updating the file ./rc/locales/default.txt, and writing its new name in the config file.