#pragma once

#include <stdint.h>
#include <stack>
#include <string>

#define MEMORY_MAX 4096
#define GPR_COUNT 16
#define KEY_COUNT 16
#define SCREEN_ROWS 32
#define SCREEN_COLUMNS 64

enum
{
    PC_START = 0x200
};

enum // General Purpose Registers
{
    V0 = 0,
    V1,
    V2,
    V3,
    V4,
    V5,
    V6,
    V7,
    V8,
    V9,
    VA,
    VB,
    VC,
    VD,
    VE,
    VF // flag register. Not to be used as a gpr.
};

enum // original keypad layout, from L -> R, Top to bottom
{
    KEY_1 = 0,
    KEY_2,
    KEY_3,
    KEY_C,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_D,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_E,
    KEY_A,
    KEY_0,
    KEY_B,
    KEY_F
};

class CHIP8
{
    uint8_t memory[MEMORY_MAX]; // 4096 locations, byte addressable
    uint8_t VReg[GPR_COUNT];
    uint16_t IReg;
    uint16_t PCReg;
    std::stack<uint16_t> stack;
    uint8_t delayTimer, soundTimer;
    uint8_t keys[KEY_COUNT];
    uint8_t screen[SCREEN_ROWS * SCREEN_COLUMNS];

public:
    void initialise();
    void loadGame(std::string);
    void emulateCycle();
    uint8_t *getMemory();
    uint8_t *getKeys();
    uint8_t *getScreen();
};
