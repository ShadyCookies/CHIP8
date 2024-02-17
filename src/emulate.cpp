#include <iomanip>
#include <iostream>

#include "chip8.hpp"

uint8_t genRandomNum() // return a random 8 bit number from 0 to 255
{
    srand(time(0));
    return rand() % 256;
}

void CHIP8::emulateCycle()
{
    // Fetch
    // Fetch must be done twice as instr is 16 bit but memory is only 8 bit wide
    // Push first byte to 15:8 position, then combine with 7:0 to get full instr
    uint16_t instrCode = (memory[PCReg] << 8) | memory[PCReg + 1];
    bool isSeqPC = true; // flag to check if we should fetch next sequential instr or not

    // Decode
    uint8_t opcodeNib = instrCode >> 12; // instrCode[15:12] gives a general idea of the instrType
    // std::cout << std::hex << int(PCReg) << " : " << std::setw(2) << std::setfill('0') << int(instrCode) << "   " << std::endl;
    // std::cout << std::hex << "V9 = " << int(VReg[V9]) << ", VA = " << int(VReg[VA]) << ", VE = " << int(VReg[VE]) << std::endl;
    switch (opcodeNib)
    {
    case 0x0:
    {
        if (instrCode == 0x00E0) // CLS - clear display
        {
            for (uint16_t i = 0; i < SCREEN_ROWS * SCREEN_COLUMNS; i++)
            {
                screen[i] = 0;
            }
        }
        else if (instrCode == 0x00EE) // RET - return from subroutine
        {
            PCReg = stack[stackPtr];
            stackPtr--;

            isSeqPC = false;
        }
    }
    break;
    case 0x1: // JUMP to location NNN
    {
        uint16_t NNN = instrCode & 0x0FFF; // NNN
        PCReg = NNN;

        isSeqPC = false;
    }
    break;
    case 0x2: // Call subroutine at NNN
    {
        uint16_t NNN = instrCode & 0x0FFF; // NNN

        stackPtr++;
        stack[stackPtr] = PCReg;
        PCReg = NNN;

        isSeqPC = false;
    }
    break;
    case 0x3: // SE - skip next instr if Vx == kk
    {
        uint8_t Vx = (instrCode >> 8) & 0x0F;
        uint8_t kk = instrCode & 0xFF;

        if (VReg[Vx] == kk)
        {
            PCReg = PCReg + 4;
            isSeqPC = false;
        }
        else
        {
            isSeqPC = true;
        }
    }
    break;
    case 0x4: // SNE - skip next instr if Vx != kk
    {
        uint8_t Vx = (instrCode >> 8) & 0x0F;
        uint8_t kk = instrCode & 0xFF;

        if (VReg[Vx] != kk)
        {
            PCReg = PCReg + 4;
            isSeqPC = false;
        }
        else
        {
            isSeqPC = true;
        }
    }
    break;
    case 0x5: // SE Vx,Vy - skip next instr if Vx == Vy
    {
        uint8_t Vx = (instrCode >> 8) & 0x0F;
        uint8_t Vy = (instrCode >> 4) & 0x00F;

        if (VReg[Vx] == VReg[Vy])
        {
            PCReg = PCReg + 4;
            isSeqPC = false;
        }
        else
        {
            isSeqPC = true;
        }
    }
    break;
    case 0x6: // LD Vx,kk - set reg[Vx] = kk
    {
        uint8_t Vx = (instrCode >> 8) & 0x0F;
        uint8_t kk = instrCode & 0x00FF;

        VReg[Vx] = kk;
    }
    break;
    case 0x7: // ADD Vx,kk - set reg[Vx] = reg[Vx] + kk
    {
        uint8_t Vx = (instrCode >> 8) & 0x0F;
        uint8_t kk = instrCode & 0x00FF;

        VReg[Vx] = VReg[Vx] + kk;
    }
    break;
    case 0x8:
    {
        uint8_t func4 = instrCode & 0x000F; // Specifies variant of 8XY? instruction
        uint8_t Vx = (instrCode >> 8) & 0x0F;
        uint8_t Vy = (instrCode >> 4) & 0x00F;

        switch (func4)
        {
        case 0x0: // LD Vx,Vy - set reg[Vx] = reg[Vy]
        {
            VReg[Vx] = VReg[Vy];
        }
        break;
        case 0x1: // OR Vx,Vy - set reg[Vx] = reg[Vx] | reg[Vy]
        {
            VReg[Vx] = VReg[Vx] | VReg[Vy];
        }
        break;
        case 0x2: // AND Vx,Vy - set reg[Vx] = reg[Vx] & reg[Vy]
        {
            VReg[Vx] = VReg[Vx] & VReg[Vy];
        }
        break;
        case 0x3: // XOR Vx,Vy - set reg[Vx] = reg[Vx] ^ reg[Vy]
        {
            VReg[Vx] = VReg[Vx] ^ VReg[Vy];
        }
        break;
        case 0x4: // ADD Vx,Vy - set reg[Vx] = reg[Vx] + reg[Vy]
        {
            VReg[VF] = (VReg[Vx] + VReg[Vy] > UINT8_MAX) ? 1 : 0; // if Vx + Vy overflows, set VF = 1, else VF = 0
            VReg[Vx] = VReg[Vx] + VReg[Vy];
        }
        break;
        case 0x5: // SUB Vx,Vy - set reg[Vx] = reg[Vx] - reg[Vy]
        {
            VReg[VF] = (VReg[Vx] - VReg[Vy] < 0) ? 1 : 0; // if Vx - Vy underflows, set VF = 1, else VF = 0
            VReg[Vx] = VReg[Vx] - VReg[Vy];
        }
        break;
        case 0x6: // SHR Vx, Vy - set  reg[Vx] = reg[Vy] then reg[Vx] >> 1;
        {
            VReg[VF] = VReg[Vy] & 0x0001; // VF = LSB of Vx
            // VReg[Vx] = VReg[Vy];
            VReg[Vx] = VReg[Vy] >> 1;
        }
        break;
        case 0x7: // SUBN Vx, Vy - set reg[Vx] = reg[Vy] - reg[Vx]
        {
            VReg[VF] = (VReg[Vy] - VReg[Vx] < 0) ? 1 : 0; // if Vy > Vx, set VF = 1, else VF = 0
            VReg[Vx] = VReg[Vy] - VReg[Vx];
        }
        break;
        case 0xE: // SHL Vx, Vy - set  reg[Vx] = reg[Vy] then reg[Vx] << 1;
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
    case 0x9: // SNE Vx,Vy - skip next instr if Vx != Vy
    {
        uint8_t Vx = (instrCode >> 8) & 0x0F;
        uint8_t Vy = (instrCode >> 4) & 0x00F;

        if (VReg[Vx] != VReg[Vy])
        {
            PCReg = PCReg + 4;
            isSeqPC = false;
        }
        else
        {
            isSeqPC = true;
        }
    }
    break;
    case 0xA: // LD I, addr - set I = addr specified by NNN
    {
        uint16_t NNN = instrCode & 0x0FFF;
        IReg = NNN;
    }
    break;
    case 0xB: // JP V0, addr - jump to addr = V0 + addr, addr given by NNN
    {
        uint16_t NNN = instrCode & 0x0FFF;
        PCReg = VReg[V0] + NNN;

        isSeqPC = false;
    }
    break;
    case 0xC: // RND Vx, byte - Set Vx = random byte AND kk.
    {
        uint8_t Vx = (instrCode >> 8) & 0x0F;
        uint8_t kk = instrCode & 0x00FF;

        VReg[Vx] = genRandomNum() & kk;
    }
    break;
    case 0xD: // DRW Vx, Vy, nibble - Display a sprite starting at memory location I at (Vx, Vy), set VF = 1 on collision.
    {
        uint8_t Vx = (instrCode >> 8) & 0x0F;
        uint8_t Vy = (instrCode >> 4) & 0x00F;
        uint8_t N = instrCode & 0x000F; // sprite width of 8 pixels, height of N pixels

        for (int rowNum = 0; rowNum < N && (VReg[Vy] + rowNum < SCREEN_ROWS); rowNum++)
        {
            uint8_t rowData = memory[IReg + rowNum];
            for (int colNum = 0; colNum < 8 && (VReg[Vx] + colNum < SCREEN_COLUMNS); colNum++)
            {
                uint8_t xCoord = VReg[Vx] + colNum;
                uint8_t yCoord = VReg[Vy] + rowNum;
                uint8_t screenWrite = (rowData >> (7 - colNum)) & 0x01;

                VReg[VF] = (screen[xCoord + yCoord * SCREEN_COLUMNS] == 1 && screenWrite) ? 1 : 0; // Set flag if pixel Overwrite
                screen[xCoord + yCoord * SCREEN_COLUMNS] ^= screenWrite;
            }
        }
    }
    break;
    case 0xE:
    {
        uint8_t func8 = instrCode & 0x00FF;
        uint8_t Vx = (instrCode >> 8) & 0x0F;

        if (func8 == 0x9E) // SKP Vx - Skip next instruction if key with the value of Vx is pressed.
        {
            // UIEventPoll(keys);

            if (keys[VReg[Vx]] != 0)
            { // nonzero

                PCReg = PCReg + 4;
                isSeqPC = false;
            }
        }
        else if (func8 == 0xA1) // SKNP Vx - Skip next instruction if key with the value of Vx is not pressed.
        {
            // UIEventPoll(keys);

            if (keys[VReg[Vx]] == 0)
            {
                PCReg = PCReg + 4;
                isSeqPC = false;
            }
        }
    }
    break;
    case 0xF:
    {
        uint8_t func8 = instrCode & 0x00FF;
        uint8_t Vx = (instrCode >> 8) & 0x0F;

        switch (func8)
        {
        case 0x07: // LD Vx, DT - Set Vx = delay timer value.
        {
            VReg[Vx] = delayTimer;
        }
        break;
        case 0x0A: //  LD Vx, K - Wait for a key press, store the value of the key in Vx.
        {
            // UIEventPoll(keys);

            for (uint8_t i = 0; i < KEY_COUNT; i++)
            {
                if (keys[i] == 1)
                {
                    VReg[Vx] = i;
                }
            }
        }
        break;
        case 0x15: // LD DT, Vx - Set delay timer = Vx.
        {
            delayTimer = VReg[Vx];
        }
        break;
        case 0x18: // LD ST, Vx - Set sound timer = Vx.
        {
            soundTimer = VReg[Vx];
        }
        break;
        case 0x1E: // LD I, Vx - Set I = I + Vx
        {
            IReg = IReg + VReg[Vx];
        }
        break;
        case 0x29: // LD F, Vx - Set I = location of sprite for digit Vx.
        {
            IReg = VReg[Vx];
        }
        break;
        case 0x33: // LD B, Vx - Store BCD representation of Vx in memory locations I, I+1, and I+2.
        {
            // Store BCD representation of value in Vx in memlocs I,I+1,I+2
            // note: BCD example: 16 -> 0x0000 0x0001 0x0110. Store ones in I, tens in I+1, hundreds in I+2
            memory[IReg] = VReg[Vx] / 100;
            memory[IReg + 1] = (VReg[Vx] / 10) % 10;
            memory[IReg + 2] = (VReg[Vx] % 100) % 10;
        }
        break;
        case 0x55: //  LD [I], Vx - Store registers V0 through Vx in memory starting at location I.
        {
            for (int i = 0; i <= Vx; i++)
            {
                memory[IReg + i] = VReg[i];
            }
        }
        break;
        case 0x65: //  LD Vx, [I] - read registers V0 through Vx in memory starting at location I.
        {
            for (int i = 0; i <= Vx; i++)
            {
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
        exit(1);
    }
    break;
    }
    // Update timers

    if (delayTimer > 0)
    {
        --delayTimer;
    }

    if (soundTimer > 0)
    {
        // BEEP!
        --soundTimer;
    }

    // for (int row = 0; row < SCREEN_ROWS; row++)
    // {
    //     for (int column = 0; column < SCREEN_COLUMNS; column++)
    //     {
    //         if ((column + row * SCREEN_COLUMNS) % SCREEN_COLUMNS == 0 && row != 0)
    //             std::cout << "\n";
    //         std::cout << int(screen[column + row * SCREEN_COLUMNS]);
    //     }
    // }

    // std::cout << "\n\n";

    if (isSeqPC == true)
        PCReg = PCReg + 2;
}