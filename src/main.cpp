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

    // setupGraphics(window, renderer, SCREEN_ROWS, SCREEN_COLUMNS);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_COLUMNS * SCALING_FACTOR, SCREEN_ROWS * SCALING_FACTOR, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, SCALING_FACTOR, SCALING_FACTOR);

    CHIP8Sys.initialise();
    CHIP8Sys.loadGame("ROMS/octojam2title.ch8");

    uint8_t *myscreen = CHIP8Sys.getScreen();

    while (true)
    {
        CHIP8Sys.emulateCycle();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        displayGraphics(renderer, myscreen, SCREEN_ROWS, SCREEN_COLUMNS);

        Sleep(5);
        SDL_RenderPresent(renderer);
    }
    closeGraphics(window);
    return 0;
}