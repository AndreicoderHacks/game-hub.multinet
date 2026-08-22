#ifndef PLATFORM_H
#define PLATFORM_H

/* ===== Ce trebuie sa ofere fiecare backend (web, nds, ps2...) ===== */

void plat_clear(void);
void plat_draw_rect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
void plat_flip(void);
unsigned int plat_time_ms(void);

/* Input: bitmask simplu, aceleasi biti pe orice platforma */
#define INPUT_P1_UP    (1 << 0)
#define INPUT_P1_DOWN  (1 << 1)
#define INPUT_P2_UP    (1 << 2)
#define INPUT_P2_DOWN  (1 << 3)
#define INPUT_START    (1 << 4)

unsigned int plat_get_input(void);

/* ===== Ce ofera jocul (core), apelat de fiecare backend din main loop ===== */

void game_init(int screen_w, int screen_h);
void game_update(float dt_seconds, unsigned int input);
void game_render(void);

#endif
