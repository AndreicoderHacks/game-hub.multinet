#include <nds.h>
#include "platform.h"

#define SCREEN_W 256
#define SCREEN_H 192

static u16 *fb;
static unsigned int input_state = 0;
static unsigned int start_ms = 0;

static inline u16 rgb15(unsigned char r, unsigned char g, unsigned char b)
{
    return RGB15(r >> 3, g >> 3, b >> 3) | BIT(15);
}

void plat_clear(void)
{
    dmaFillHalfWords(0, fb, SCREEN_W * SCREEN_H * 2);
}

void plat_draw_rect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b)
{
    u16 color = rgb15(r, g, b);
    for (int yy = y; yy < y + h; yy++) {
        if (yy < 0 || yy >= SCREEN_H) continue;
        for (int xx = x; xx < x + w; xx++) {
            if (xx < 0 || xx >= SCREEN_W) continue;
            fb[yy * SCREEN_W + xx] = color;
        }
    }
}

void plat_flip(void)
{
    /* single bitmap background, nimic de schimbat intre buffere aici */
}

unsigned int plat_time_ms(void)
{
    return (cpuGetTiming() / BUS_CLOCK) * 1000 - start_ms;
}

unsigned int plat_get_input(void)
{
    scanKeys();
    int keys = keysHeld();
    unsigned int in = 0;
    if (keys & KEY_UP)    in |= INPUT_P1_UP;
    if (keys & KEY_DOWN)  in |= INPUT_P1_DOWN;
    if (keys & KEY_X)     in |= INPUT_P2_UP;
    if (keys & KEY_B)     in |= INPUT_P2_DOWN;
    return in;
}

int main(void)
{
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    fb = bgGetGfxPtr(bg);

    start_ms = (cpuGetTiming() / BUS_CLOCK) * 1000;
    game_init(SCREEN_W, SCREEN_H);

    unsigned int last = 0;
    while (1) {
        unsigned int now = plat_time_ms();
        float dt = last ? (now - last) / 1000.0f : 0.0f;
        last = now;

        unsigned int input = plat_get_input();
        game_update(dt, input);
        game_render();

        swiWaitForVBlank();
    }
    return 0;
}
