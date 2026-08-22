#include "platform.h"

static int SCREEN_W = 320;
static int SCREEN_H = 240;

#define PADDLE_W 8
#define PADDLE_H 40
#define BALL_SIZE 6
#define PADDLE_SPEED 160.0f  /* px/s */
#define BALL_SPEED 140.0f

static float p1_y, p2_y;
static float ball_x, ball_y;
static float ball_vx, ball_vy;
static int score1, score2;

static void reset_ball(int dir)
{
    ball_x = SCREEN_W / 2 - BALL_SIZE / 2;
    ball_y = SCREEN_H / 2 - BALL_SIZE / 2;
    ball_vx = BALL_SPEED * dir;
    ball_vy = BALL_SPEED * 0.5f;
}

void game_init(int screen_w, int screen_h)
{
    SCREEN_W = screen_w;
    SCREEN_H = screen_h;
    p1_y = SCREEN_H / 2 - PADDLE_H / 2;
    p2_y = SCREEN_H / 2 - PADDLE_H / 2;
    score1 = 0;
    score2 = 0;
    reset_ball(1);
}

void game_update(float dt, unsigned int input)
{
    if (input & INPUT_P1_UP)   p1_y -= PADDLE_SPEED * dt;
    if (input & INPUT_P1_DOWN) p1_y += PADDLE_SPEED * dt;
    if (input & INPUT_P2_UP)   p2_y -= PADDLE_SPEED * dt;
    if (input & INPUT_P2_DOWN) p2_y += PADDLE_SPEED * dt;

    if (p1_y < 0) p1_y = 0;
    if (p1_y > SCREEN_H - PADDLE_H) p1_y = SCREEN_H - PADDLE_H;
    if (p2_y < 0) p2_y = 0;
    if (p2_y > SCREEN_H - PADDLE_H) p2_y = SCREEN_H - PADDLE_H;

    ball_x += ball_vx * dt;
    ball_y += ball_vy * dt;

    if (ball_y < 0 || ball_y > SCREEN_H - BALL_SIZE) ball_vy = -ball_vy;

    /* paddle 1 (stanga) */
    if (ball_x < PADDLE_W && ball_y + BALL_SIZE > p1_y && ball_y < p1_y + PADDLE_H) {
        ball_vx = -ball_vx;
        ball_x = PADDLE_W;
    }
    /* paddle 2 (dreapta) */
    if (ball_x > SCREEN_W - PADDLE_W - BALL_SIZE && ball_y + BALL_SIZE > p2_y && ball_y < p2_y + PADDLE_H) {
        ball_vx = -ball_vx;
        ball_x = SCREEN_W - PADDLE_W - BALL_SIZE;
    }

    if (ball_x < 0) { score2++; reset_ball(1); }
    if (ball_x > SCREEN_W) { score1++; reset_ball(-1); }
}

void game_render(void)
{
    plat_clear();
    plat_draw_rect(0, (int)p1_y, PADDLE_W, PADDLE_H, 255, 255, 255);
    plat_draw_rect(SCREEN_W - PADDLE_W, (int)p2_y, PADDLE_W, PADDLE_H, 255, 255, 255);
    plat_draw_rect((int)ball_x, (int)ball_y, BALL_SIZE, BALL_SIZE, 255, 255, 255);
    plat_flip();
}
