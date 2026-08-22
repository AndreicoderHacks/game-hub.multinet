#include <SDL2/SDL.h>
#include <emscripten.h>
#include "platform.h"

#define SCREEN_W 320
#define SCREEN_H 240

static SDL_Window *window;
static SDL_Renderer *renderer;
static unsigned int input_state = 0;

void plat_clear(void)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void plat_draw_rect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b)
{
    SDL_Rect rect = { x, y, w, h };
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void plat_flip(void)
{
    SDL_RenderPresent(renderer);
}

unsigned int plat_time_ms(void)
{
    return SDL_GetTicks();
}

unsigned int plat_get_input(void)
{
    return input_state;
}

static void poll_input(void)
{
    SDL_Event e;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) emscripten_cancel_main_loop();
    }
    input_state = 0;
    if (keys[SDL_SCANCODE_W]) input_state |= INPUT_P1_UP;
    if (keys[SDL_SCANCODE_S]) input_state |= INPUT_P1_DOWN;
    if (keys[SDL_SCANCODE_UP]) input_state |= INPUT_P2_UP;
    if (keys[SDL_SCANCODE_DOWN]) input_state |= INPUT_P2_DOWN;
}

static void main_loop(void)
{
    static unsigned int last = 0;
    unsigned int now = plat_time_ms();
    float dt = last ? (now - last) / 1000.0f : 0.0f;
    last = now;

    poll_input();
    game_update(dt, input_state);
    game_render();
}

int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Game Hub - Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               SCREEN_W, SCREEN_H, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    game_init(SCREEN_W, SCREEN_H);

    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}
