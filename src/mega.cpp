#include <stdio.h>
#include <iostream>
#include <limits.h>
#include <time.h>
#include <windows.h>
#include <fstream>
#include <cstring>

//SDL bullshit
#define SDL_MAIN_HANDLED
#include "SDL_src/include/SDL2/SDL.h"
#include <iomanip>

#define MEMORY_MAX 4096
#define GPR_COUNT 16
#define STACK_COUNT 16
#define KEY_COUNT 16
#define SCREEN_ROWS 64
#define SCREEN_COLUMNS 48
#define REFRESH_RATE 60

enum
{
    PC_START = 0x200
};

enum    // General Purpose Registers
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

enum    // original keypad layout, from L -> R, Top to bottom
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

int KEYBINDS[KEY_COUNT] = {
    SDL_SCANCODE_1,SDL_SCANCODE_2,SDL_SCANCODE_3,SDL_SCANCODE_4,
    SDL_SCANCODE_Q,SDL_SCANCODE_W,SDL_SCANCODE_E,SDL_SCANCODE_R,
    SDL_SCANCODE_A,SDL_SCANCODE_S,SDL_SCANCODE_D,SDL_SCANCODE_F,
    SDL_SCANCODE_Z,SDL_SCANCODE_X,SDL_SCANCODE_C,SDL_SCANCODE_V,  
};

void UIEventPoll(uint8_t *keys);
uint8_t genRandomNum();
void displayGraphics(uint8_t *screen);
void closeGraphics();

class CHIP8
{
    uint8_t     memory[MEMORY_MAX]; // 4096 locations, byte addressable 
    uint8_t     VReg[GPR_COUNT];
    uint16_t    IReg;               
    uint16_t    PCReg;              
    uint16_t    stack[STACK_COUNT];
    uint8_t     stackPtr;
    uint8_t     delayTimer, soundTimer;
    uint8_t     keys[KEY_COUNT];
    uint8_t     screen[SCREEN_ROWS * SCREEN_COLUMNS];

    public:
        void initialise();
        void loadGame( char *ROM );
        void emulateCycle();
        uint8_t *getMemory();
        uint8_t *getKeys();
        uint8_t *getScreen();
}CHIP8Sys;

void CHIP8::initialise()
{

    PCReg = PC_START;
    IReg = 0;
    stackPtr = 0;

    // Clear display
    for (uint16_t i = 0; i < SCREEN_ROWS * SCREEN_COLUMNS; i++) {
        screen[i] = 0;
    }
    // Clear stack
    for (uint16_t i = 0; i < STACK_COUNT; i++) {
        stack[i] = 0;
    }
    // Clear registers V0-VF
    for (uint16_t i = 0; i < GPR_COUNT; i++) {
        VReg[i] = 0;
    }
    // Clear memory
    for (uint16_t i = 0; i < MEMORY_MAX; i++) {
        memory[i] = 0;
    }
    // Clear keys
    for (uint16_t i = 0; i < KEY_COUNT; i++) {
        keys[i] = 0;
    }
    // Load fontset at memory location 0x050 onwards
    for (uint16_t i = 0; i < 80; ++i){
        memory[0x050 + i] = FONTSET[i];
    }
    // Reset timers
    delayTimer = 0;
    soundTimer = 0;
}

uint8_t * CHIP8::getMemory()
{
    return memory;
}

uint8_t * CHIP8::getKeys()
{
    return keys;
}

uint8_t * CHIP8::getScreen()
{
    return screen;
}

void CHIP8::loadGame( char *ROM )
{
    std::ifstream gameBinary(ROM, std::ios::binary); 
    std::ofstream gameHex("hexCodes.txt", std::ios::binary);

    char byte;
    uint16_t memPtr = PC_START;

    while (gameBinary.get(byte)) {
        memory[memPtr] = byte;
        gameHex << std::hex << std::setw(2) << std::setfill('0') << int(memory[memPtr]);
        if(memPtr % 2 == 1) {
            gameHex << "\n";
        }
        memPtr++;
    }

    gameBinary.close(); 
    gameHex.close();
}

void CHIP8::emulateCycle()
{
    // Fetch 
    // Fetch must be done twice as instr is 16 bit but memory is only 8 bit wide
    // Push first byte to 15:8 position, then combine with 7:0 to get full instr 
    uint16_t instrCode  = (memory[PCReg] << 8) | memory[PCReg+1];
    bool     isSeqPC    = true;     // flag to check if we should fetch next sequential instr or not

    // Decode 
    uint8_t opcodeNib   = instrCode >> 12;   // instrCode[15:12] gives a general idea of the instrType
    std::cout << std::hex << int(PCReg) << " : " << std::setw(2) << std::setfill('0') << int(instrCode) << "   " << std::endl;;
    // std::cout << std::hex << "V9 = " << int(VReg[V9]) << ", VA = " << int(VReg[VA]) << ", VE = " << int(VReg[VE]) << std::endl;
    switch (opcodeNib)
    {
        case 0x0:
            {
                if (instrCode == 0x00E0)        // CLS - clear display
                {
                    for (uint16_t i = 0; i < SCREEN_ROWS * SCREEN_COLUMNS; i++) {
                        screen[i] = 0;
                    }
                }
                else if (instrCode == 0x00EE)   // RET - return from subroutine
                {
                    PCReg = stack[stackPtr];
                    stackPtr--;

                    isSeqPC = false;
                }
            }
            break;
        case 0x1:   // JUMP to location NNN 
            {
                uint16_t NNN = instrCode & 0x0FFF;  // NNN
                PCReg = NNN;

                isSeqPC = false;
            }
            break;
        case 0x2:   // Call subroutine at NNN
            {
                uint16_t NNN = instrCode & 0x0FFF;  // NNN

                stackPtr++;
                stack[stackPtr] = PCReg;
                PCReg = NNN;

                isSeqPC = false;
            }
            break;
        case 0x3:   // SE - skip next instr if Vx == kk
            {
                uint8_t Vx = (instrCode >> 8) & 0x0F; 
                uint8_t kk = instrCode & 0xFF;

                if(VReg[Vx] == kk)
                {
                    PCReg = PCReg+4;
                    isSeqPC = false;    
                }
                else
                {
                    isSeqPC = true;
                }
            }
            break;
        case 0x4:   // SNE - skip next instr if Vx != kk
            {
                uint8_t Vx = (instrCode >> 8) & 0x0F; 
                uint8_t kk = instrCode & 0xFF;

                if(VReg[Vx] != kk)
                {
                    PCReg = PCReg+4;
                    isSeqPC = false;    
                }
                else
                {
                    isSeqPC = true;
                }               
            }
            break;
        case 0x5:   // SE Vx,Vy - skip next instr if Vx == Vy
            {
                uint8_t Vx = (instrCode >> 8) & 0x0F; 
                uint8_t Vy = (instrCode >> 4) & 0x00F; 

                if(VReg[Vx] == VReg[Vy]) {
                    PCReg = PCReg+4;
                    isSeqPC = false;    
                }
                else {
                    isSeqPC = true;
                }
            }
            break;
        case 0x6:   // LD Vx,kk - set reg[Vx] = kk
            {
                uint8_t Vx = (instrCode >> 8) & 0x0F; 
                uint8_t kk = instrCode & 0x00FF;

                VReg[Vx] = kk;                
            }
            break;
        case 0x7:   // ADD Vx,kk - set reg[Vx] = reg[Vx] + kk
            {
                uint8_t Vx = (instrCode >> 8) & 0x0F; 
                uint8_t kk = instrCode & 0x00FF;

                VReg[Vx] = VReg[Vx] + kk;                
            }
            break;
        case 0x8:
            {
                uint8_t func4   = instrCode & 0x000F;       // Specifies variant of 8XY? instruction
                uint8_t Vx      = (instrCode >> 8) & 0x0F; 
                uint8_t Vy      = (instrCode >> 4) & 0x00F; 

                switch (func4)
                {
                    case 0x0:   // LD Vx,Vy - set reg[Vx] = reg[Vy]
                        {
                            VReg[Vx] = VReg[Vy];
                        }
                        break;
                    case 0x1:   // OR Vx,Vy - set reg[Vx] = reg[Vx] | reg[Vy]
                        {
                            VReg[Vx] = VReg[Vx] | VReg[Vy];
                        }
                        break;
                    case 0x2:   // AND Vx,Vy - set reg[Vx] = reg[Vx] & reg[Vy]
                        {
                            VReg[Vx] = VReg[Vx] & VReg[Vy];
                        }
                        break;
                    case 0x3:   // XOR Vx,Vy - set reg[Vx] = reg[Vx] ^ reg[Vy]
                        {
                            VReg[Vx] = VReg[Vx] ^ VReg[Vy];
                        }
                        break;
                    case 0x4:   // ADD Vx,Vy - set reg[Vx] = reg[Vx] + reg[Vy]
                        {
                            VReg[VF] = (VReg[Vx] + VReg[Vy] > UINT8_MAX)? 1 : 0;    // if Vx + Vy overflows, set VF = 1, else VF = 0
                            VReg[Vx] = VReg[Vx] + VReg[Vy];  
                        }
                        break;
                    case 0x5:   // SUB Vx,Vy - set reg[Vx] = reg[Vx] - reg[Vy]
                        {
                            VReg[VF] = (VReg[Vx] - VReg[Vy] < 0)? 1 : 0;    // if Vx - Vy underflows, set VF = 1, else VF = 0
                            VReg[Vx] = VReg[Vx] - VReg[Vy];   
                        }
                        break;
                    case 0x6:   // SHR Vx, Vy - set  reg[Vx] = reg[Vy] then reg[Vx] >> 1;
                        {
                            VReg[VF] = VReg[Vy] & 0x0001;   // VF = LSB of Vx
                            // VReg[Vx] = VReg[Vy];
                            VReg[Vx] = VReg[Vy] >> 1;
                        }
                        break;
                    case 0x7:   // SUBN Vx, Vy - set reg[Vx] = reg[Vy] - reg[Vx]
                        {
                            VReg[VF] = (VReg[Vy] - VReg[Vx] < 0)? 1 : 0;    // if Vy > Vx, set VF = 1, else VF = 0
                            VReg[Vx] = VReg[Vy] - VReg[Vx];   
                        }
                        break;
                    case 0xE:   // SHL Vx, Vy - set  reg[Vx] = reg[Vy] then reg[Vx] << 1;
                        {
                            VReg[VF] = (VReg[Vy] >> 15) & 0x1; // VF = MSB of Vx
                            // VReg[Vx] = VReg[Vy];
                            VReg[Vx] = VReg[Vy] << 1;
                        }
                        break;
                    default: 
                        break;
                }
            }
            break;
        case 0x9:   // SNE Vx,Vy - skip next instr if Vx != Vy
            {
                uint8_t Vx = (instrCode >> 8) & 0x0F; 
                uint8_t Vy = (instrCode >> 4) & 0x00F; 

                if(VReg[Vx] != VReg[Vy]) {
                    PCReg = PCReg+4;
                    isSeqPC = false;    
                }
                else{
                    isSeqPC = true;
                }
            }                
            break;
        case 0xA:   // LD I, addr - set I = addr specified by NNN
            {
                uint16_t NNN    = instrCode & 0x0FFF;
                IReg            = NNN;
            }
            break;
        case 0xB:   // JP V0, addr - jump to addr = V0 + addr, addr given by NNN
            {
                uint16_t NNN    = instrCode & 0x0FFF;
                PCReg           = VReg[V0] + NNN;

                isSeqPC = false; 
            }
            break;
        case 0xC:   // RND Vx, byte - Set Vx = random byte AND kk.
            {
                uint8_t Vx = (instrCode >> 8) & 0x0F;
                uint8_t kk = instrCode & 0x00FF;

                VReg[Vx] = genRandomNum() & kk;           
            }
            break;
        case 0xD:   // DRW Vx, Vy, nibble - Display a sprite starting at memory location I at (Vx, Vy), set VF = 1 on collision.
            {
                uint8_t Vx  = (instrCode >> 8) & 0x0F; 
                uint8_t Vy  = (instrCode >> 4) & 0x00F;   
                uint8_t N   = instrCode & 0x000F;       // sprite width of 8 pixels, height of N pixels           

                for(int rowNum = 0; rowNum < N && (VReg[Vy] + rowNum < SCREEN_ROWS) ; rowNum++) {
                    uint8_t rowData = memory[IReg + rowNum];    
                    for(int colNum = 0; colNum < 8 && (VReg[Vx] + colNum < SCREEN_COLUMNS); colNum++) {
                        uint8_t xCoord = VReg[Vx] + colNum;
                        uint8_t yCoord = VReg[Vy] + rowNum;
                        uint8_t screenWrite = (rowData >> (7 - colNum)) & 0x01;

                        VReg[VF] = (screen[xCoord + yCoord * SCREEN_COLUMNS] == 1 && screenWrite)? 1: 0; // Set flag if pixel Overwrite
                        screen[xCoord + yCoord * SCREEN_COLUMNS] ^= screenWrite;
                    } 
                }
            }
            break;
        case 0xE:
            {
                uint8_t func8 = instrCode & 0x00FF;
                uint8_t Vx  = (instrCode >> 8) & 0x0F;

                if (func8 == 0x9E)  // SKP Vx - Skip next instruction if key with the value of Vx is pressed.
                { 
                    UIEventPoll(keys);

                    if(keys[VReg[Vx]] != 0) {   // nonzero

                        PCReg = PCReg + 4;
                        isSeqPC = false;
                    }
                }
                else if (func8 == 0xA1) // SKNP Vx - Skip next instruction if key with the value of Vx is not pressed.
                {
                    UIEventPoll(keys);

                    if(keys[VReg[Vx]] == 0) {
                        PCReg = PCReg + 4;
                        isSeqPC = false;
                    }                
                }
            }
            break;
        case 0xF:
            {
                uint8_t func8   = instrCode & 0x00FF;
                uint8_t Vx      = (instrCode >> 8) & 0x0F;

                switch (func8)
                {
                    case 0x07:  // LD Vx, DT - Set Vx = delay timer value.
                        {
                            VReg[Vx] = delayTimer;
                        }
                        break;
                    case 0x0A:  //  LD Vx, K - Wait for a key press, store the value of the key in Vx.
                        {
                            UIEventPoll(keys);

                            for(uint8_t i = 0; i < KEY_COUNT; i++) {
                                if(keys[i] == 1) {
                                    VReg[Vx] = i;
                                }
                            }
                        }
                        break;   
                    case 0x15:  // LD DT, Vx - Set delay timer = Vx.
                        {
                            delayTimer = VReg[Vx];
                        }
                        break;                    
                    case 0x18:  // LD ST, Vx - Set sound timer = Vx.
                        {
                            soundTimer = VReg[Vx];
                        }
                        break;                    
                    case 0x1E:  // LD I, Vx - Set I = I + Vx
                        {
                            IReg = IReg + VReg[Vx];
                        }
                        break;
                    case 0x29:  // LD F, Vx - Set I = location of sprite for digit Vx.
                        {
                            IReg = VReg[Vx];
                        }
                        break;
                    case 0x33:  // LD B, Vx - Store BCD representation of Vx in memory locations I, I+1, and I+2.
                        {
                            // Store BCD representation of value in Vx in memlocs I,I+1,I+2
                            // note: BCD example: 16 -> 0x0000 0x0001 0x0110. Store ones in I, tens in I+1, hundreds in I+2
                            memory[IReg]     = VReg[Vx] / 100;
                            memory[IReg + 1] = (VReg[Vx] / 10) % 10;
                            memory[IReg + 2] = (VReg[Vx] % 100) % 10;
                        }
                        break;
                    case 0x55:  //  LD [I], Vx - Store registers V0 through Vx in memory starting at location I.
                        {
                            for(int i = 0; i <= Vx; i++){
                                memory[IReg + i] = VReg[i];
                            }
                        }
                        break;
                    case 0x65:  //  LD Vx, [I] - read registers V0 through Vx in memory starting at location I.
                        {
                            for(int i = 0; i <= Vx; i++){
                                VReg[i] = memory[IReg + i];
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        default: 
            {
                std::cout << "unknown instruction " << std::hex << instrCode;
            }
            break;
    }
    // Update timers  

    if(delayTimer > 0){
        --delayTimer;
    }
 
    if(soundTimer > 0){
       // BEEP!
        --soundTimer;
    }  

    // for(int row = 0; row < SCREEN_ROWS; row++)
    // {
    //     for(int column = 0; column < SCREEN_COLUMNS; column++)
    //     {
    //         if((column + row * SCREEN_COLUMNS) % SCREEN_COLUMNS == 0 && row != 0) std::cout << "\n";
    //         std::cout << int(screen[column + row * SCREEN_COLUMNS]);
    //     }
    // }

    // std::cout << "\n\n";

    if(isSeqPC == true) PCReg = PCReg + 2;    
}


#define SCALING_FACTOR 16

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

void UIEventPoll(uint8_t *keys)
{
    SDL_Event UIEvent;

    while(SDL_PollEvent(&UIEvent) > 0) {
        switch(UIEvent.type){
            case SDL_QUIT:
                {
                    exit(0);
                }
                break;
            case SDL_KEYDOWN:
                {
                    for(uint8_t i = 0; i < KEY_COUNT; i++){
                        if(UIEvent.key.keysym.scancode == KEYBINDS[i]) {
                            keys[i] = 1;
                        }
                    }
                }
                break;
            case SDL_KEYUP:
                {
                    for(uint8_t i = 0; i < KEY_COUNT; i++){
                        if(UIEvent.key.keysym.scancode == KEYBINDS[i]) {
                            keys[i] = 0;
                        }
                    }
                }
                break;
            default:
                break;
        }
    }
}


uint8_t genRandomNum()  // return a random 8 bit number from 0 to 255
{
    srand(time(0));
    return rand() % 256;
}


void displayGraphics(uint8_t *screen)
{
    for(int i = 0; i < SCREEN_ROWS * SCREEN_COLUMNS; i++) {
        int drawColor = screen[i];
        // SDL_SetRenderDrawColor(renderer, 255*drawColor, 255*drawColor, 255*drawColor, 255);
        if(drawColor) {
            int xCoord = i % SCREEN_COLUMNS;
            int yCoord = i / SCREEN_COLUMNS;
            SDL_RenderDrawPoint(renderer, xCoord, yCoord);
        }
    }
}

void setupGraphics()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_ROWS*16,SCREEN_COLUMNS*16,0,&window,&renderer);
    SDL_RenderSetScale(renderer,SCALING_FACTOR,SCALING_FACTOR);
}

void closeGraphics()
{
    SDL_DestroyWindow(window);
    window = NULL;

    std::cout << "Quitting SDL";
    SDL_Quit();
    std::cout << "CHIP8 Closing";
}

int main()
{
    // char gameRom[50] = "octojam1title.ch8";

    setupGraphics();


    CHIP8Sys.initialise();
    CHIP8Sys.loadGame("ROMS/output_fixed.ch8");

    uint8_t *myscreen = CHIP8Sys.getScreen();

    while(true)
    {
        CHIP8Sys.emulateCycle();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);   

        displayGraphics(CHIP8Sys.getScreen());

        SDL_RenderPresent(renderer);
        Sleep(50);

        // for(int i = 0; i < SCREEN_ROWS * SCREEN_COLUMNS; i++)
        // {
        //     myscreen[i] = abs(1-myscreen[i]);
        // }
    }
    closeGraphics();
    return 0;
}