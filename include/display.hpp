#define SDL_MAIN_HANDLED
#include <SDL.h>

#define REFRESH_RATE 60
#define SCALING_FACTOR 16
#define KEYBIND_COUNT 16

void UIEventPoll(uint8_t *keys);
void displayGraphics(SDL_Renderer *renderer, uint8_t *screen, int nRows, int nColumns);
void closeGraphics(SDL_Window *window);
void setupGraphics(SDL_Window *window, SDL_Renderer *renderer, int nRows, int nColumns);