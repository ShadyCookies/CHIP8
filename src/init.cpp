#include <fstream>
#include <iomanip>

#include "chip8.hpp"

uint8_t FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void CHIP8::initialise()
{

    PCReg = PC_START;
    IReg = 0;
    // stackPtr = 0;

    // Clear display
    for (uint16_t i = 0; i < SCREEN_ROWS * SCREEN_COLUMNS; i++)
    {
        screen[i] = 0;
    }
    // Clear registers V0-VF
    for (uint16_t i = 0; i < GPR_COUNT; i++)
    {
        VReg[i] = 0;
    }
    // Clear memory
    for (uint16_t i = 0; i < MEMORY_MAX; i++)
    {
        memory[i] = 0;
    }
    // Clear keys
    for (uint16_t i = 0; i < KEY_COUNT; i++)
    {
        keys[i] = 0;
    }
    // Load fontset at memory location 0x050 onwards
    for (uint16_t i = 0; i < 80; ++i)
    {
        memory[0x050 + i] = FONTSET[i];
    }
    // Reset timers
    delayTimer = 0;
    soundTimer = 0;
}

uint8_t *CHIP8::getMemory()
{
    return memory;
}

uint8_t *CHIP8::getKeys()
{
    return keys;
}

uint8_t *CHIP8::getScreen()
{
    return screen;
}

void CHIP8::loadGame(char *ROM)
{
    std::ifstream gameBinary(ROM, std::ios::binary);
    std::ofstream gameHex("hexCodes.txt", std::ios::binary); // Only for diagnostics, not used in the program

    char byte;
    uint16_t memPtr = PC_START;

    while (gameBinary.get(byte))
    {
        memory[memPtr] = byte;
        if (memPtr % 2 == 0)
            gameHex << std::hex << memPtr << " : ";
        gameHex << std::hex << std::setw(2) << std::setfill('0') << int(memory[memPtr]);
        if (memPtr % 2 == 1)
        {
            gameHex << "\n";
        }
        memPtr++;
    }

    gameBinary.close();
    gameHex.close();
}