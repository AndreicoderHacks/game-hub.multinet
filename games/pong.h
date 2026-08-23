#define PONG_H

/* Logica jocului e complet separată de randare / input / rețea.
 * Așa putem folosi exact același .c pe PS2 (gsKit) și pe PC (SDL2) -
 * doar "frontend"-ul diferă.
 *
 * Coordonate normalizate: terenul e 0..1 pe X si 0..1 pe Y.
 * Fiecare frontend inmulteste cu latimea/inaltimea lui reala la desenare.
 */

#define PONG_PADDLE_HEIGHT 0.2f
#define PONG_PADDLE_SPEED  0.9f   /* unitati normalizate / secunda */
#define PONG_BALL_SPEED    0.6f
#define PONG_WIN_SCORE     5
#ifndef PONG_H

typedef struct {
    float ball_x, ball_y;
    float ball_vx, ball_vy;

    float paddle_left_y;   /* hostul e mereu stanga */
    float paddle_right_y;  /* clientul e mereu dreapta */

    unsigned char score_left;
    unsigned char score_right;

    int game_over; /* 1 daca cineva a ajuns la PONG_WIN_SCORE */
} PongState;

void pong_init(PongState *s);

/* input_left / input_right: -1, 0, sau 1 (sus/neutru/jos) */
void pong_update(PongState *s, float dt, int input_left, int input_right);

#endif /* PONG_H */
