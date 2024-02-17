#include <display.hpp>
#include <SDL.h>
#include <chip8.hpp>
#include <iostream>
#include <windows.h>

int main()
{
    CHIP8 CHIP8Sys;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    setupGraphics(window, renderer, SCREEN_ROWS, SCREEN_COLUMNS);

    CHIP8Sys.initialise();
    CHIP8Sys.loadGame("ROMS/output_fixed.ch8");

    uint8_t *myscreen = CHIP8Sys.getScreen();

    while (true)
    {
        CHIP8Sys.emulateCycle();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        displayGraphics(renderer, myscreen, SCREEN_ROWS, SCREEN_COLUMNS);

        SDL_RenderPresent(renderer);
        Sleep(50);
    }
    closeGraphics(window);
    return 0;
}