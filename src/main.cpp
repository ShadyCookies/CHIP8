#include <display.hpp>
#include <SDL.h>
#include <chip8.hpp>
#include <iostream>
#include <windows.h>
#include <string>

int main()
{
    CHIP8 CHIP8Sys;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    std::string ROM = "ROMS/flightrunner_new.ch8";

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(SCREEN_COLUMNS * SCALING_FACTOR, SCREEN_ROWS * SCALING_FACTOR, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, SCALING_FACTOR, SCALING_FACTOR);

    CHIP8Sys.initialise();
    CHIP8Sys.loadGame(ROM);

    uint8_t *myscreen = CHIP8Sys.getScreen();

    while (true)
    {

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

        CHIP8Sys.emulateCycle();
        UIEventPoll(CHIP8Sys.getKeys());
        displayGraphics(renderer, myscreen, SCREEN_ROWS, SCREEN_COLUMNS);

        SDL_RenderPresent(renderer);
        SDL_Delay(2);
    }
    closeGraphics(window);
    return 0;
}