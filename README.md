# CHIP8 Emulator

CHIP8 Emulator developed in C++, with SDL2 for Graphics and Keyboard support. Currently does not support sound, but everything else should work fine!

### Running the Emulator:
----------------------
Copy the entire project to a folder. Any ROM you wish to run must be in the .ch8 format and be placed in the ROMS/ folder. Then edit the path in main.cpp ( I really should make this a commandline argument.. ) and run:

```cpp
  make run  
```
The generated main.exe will be located in the bin/ folder.

### Sample Roms:
- OctoJam Title:

![Alt Text](https://media.giphy.com/media/xYPBKti7x4O7sqBXWH/giphy.gif)

- Flight Game:

![Alt Text](https://media.giphy.com/media/v1.Y2lkPTc5MGI3NjExZm9sN3h0OG1nZzhuM3VicHRzNjBsMXo4Z3U4dWlkbzg0NjY1cXRyZyZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/SaEnwPJvTCdzXfOA8D/giphy.gif)
