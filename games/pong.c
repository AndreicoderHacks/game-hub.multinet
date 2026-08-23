#include "pong.h"
#include <stdlib.h>

static void reset_ball(PongState *s, int dir)
{
    s->ball_x = 0.5f;
    s->ball_y = 0.5f;
    s->ball_vx = PONG_BALL_SPEED * dir;
    /* mica variatie pe verticala ca sa nu fie mereu identic */
    s->ball_vy = PONG_BALL_SPEED * (((rand() % 100) - 50) / 100.0f);
}

void pong_init(PongState *s)
{
    s->paddle_left_y = 0.5f - PONG_PADDLE_HEIGHT / 2.0f;
    s->paddle_right_y = 0.5f - PONG_PADDLE_HEIGHT / 2.0f;
    s->score_left = 0;
    s->score_right = 0;
    s->game_over = 0;
    reset_ball(s, 1);
}

void pong_update(PongState *s, float dt, int input_left, int input_right)
{
    if (s->game_over)
        return;

    /* miscare padele */
    s->paddle_left_y += input_left * PONG_PADDLE_SPEED * dt;
    s->paddle_right_y += input_right * PONG_PADDLE_SPEED * dt;

    if (s->paddle_left_y < 0.0f) s->paddle_left_y = 0.0f;
    if (s->paddle_left_y > 1.0f - PONG_PADDLE_HEIGHT) s->paddle_left_y = 1.0f - PONG_PADDLE_HEIGHT;
    if (s->paddle_right_y < 0.0f) s->paddle_right_y = 0.0f;
    if (s->paddle_right_y > 1.0f - PONG_PADDLE_HEIGHT) s->paddle_right_y = 1.0f - PONG_PADDLE_HEIGHT;

    /* miscare minge */
    s->ball_x += s->ball_vx * dt;
    s->ball_y += s->ball_vy * dt;

    if (s->ball_y < 0.0f) { s->ball_y = 0.0f; s->ball_vy = -s->ball_vy; }
    if (s->ball_y > 1.0f) { s->ball_y = 1.0f; s->ball_vy = -s->ball_vy; }

    /* coliziune padela stanga */
    if (s->ball_x <= 0.02f &&
        s->ball_y >= s->paddle_left_y &&
        s->ball_y <= s->paddle_left_y + PONG_PADDLE_HEIGHT &&
        s->ball_vx < 0)
    {
        s->ball_vx = -s->ball_vx;
        s->ball_x = 0.02f;
    }

    /* coliziune padela dreapta */
    if (s->ball_x >= 0.98f &&
        s->ball_y >= s->paddle_right_y &&
        s->ball_y <= s->paddle_right_y + PONG_PADDLE_HEIGHT &&
        s->ball_vx > 0)
    {
        s->ball_vx = -s->ball_vx;
        s->ball_x = 0.98f;
    }

    /* punct pentru dreapta */
    if (s->ball_x < 0.0f) {
        s->score_right++;
        if (s->score_right >= PONG_WIN_SCORE) s->game_over = 1;
        else reset_ball(s, 1);
    }

    /* punct pentru stanga */
    if (s->ball_x > 1.0f) {
        s->score_left++;
        if (s->score_left >= PONG_WIN_SCORE) s->game_over = 1;
        else reset_ball(s, -1);
    }
}
