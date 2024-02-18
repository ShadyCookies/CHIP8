#include <display.hpp>
#include <SDL.h>
#include <iostream>

int KEYBINDS[KEYBIND_COUNT] = {
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_4,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_W,
    SDL_SCANCODE_E,
    SDL_SCANCODE_R,
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_D,
    SDL_SCANCODE_F,
    SDL_SCANCODE_Z,
    SDL_SCANCODE_X,
    SDL_SCANCODE_C,
    SDL_SCANCODE_V,
};

void UIEventPoll(uint8_t *keys)
{
    SDL_Event UIEvent;

    while (SDL_PollEvent(&UIEvent) > 0)
    {
        switch (UIEvent.type)
        {
        case SDL_QUIT:
        {
            exit(0);
        }
        break;
        case SDL_KEYDOWN:
        {
            for (uint8_t i = 0; i < KEYBIND_COUNT; i++)
            {
                if (UIEvent.key.keysym.scancode == KEYBINDS[i])
                {
                    keys[i] = 1;
                }
            }
        }
        break;
        case SDL_KEYUP:
        {
            for (uint8_t i = 0; i < KEYBIND_COUNT; i++)
            {
                if (UIEvent.key.keysym.scancode == KEYBINDS[i])
                {
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

void displayGraphics(SDL_Renderer *renderer, uint8_t *screen, int nRows, int nColumns)
{
    for (int i = 0; i < nRows * nColumns; i++)
    {
        int drawColor = screen[i];
        if (drawColor)
        {
            int xCoord = i % nColumns;
            int yCoord = i / nColumns;
            SDL_RenderDrawPoint(renderer, xCoord, yCoord);
        }
    }
}

// void setupGraphics(SDL_Window *window, SDL_Renderer *renderer, int nRows, int nColumns)
// {
//     SDL_Init(SDL_INIT_VIDEO);
//     SDL_CreateWindowAndRenderer(nColumns * SCALING_FACTOR, nRows * SCALING_FACTOR, 0, &window, &renderer);
//     SDL_RenderSetScale(renderer, SCALING_FACTOR, SCALING_FACTOR);
// }

void closeGraphics(SDL_Window *window)
{
    SDL_DestroyWindow(window);
    window = NULL;

    std::cout << "Quitting SDL";
    SDL_Quit();
    std::cout << "CHIP8 Closing";
}