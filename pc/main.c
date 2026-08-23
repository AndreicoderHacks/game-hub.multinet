/* Hub PC (SDL2) - deocamdata doar Pong.
 *
 * Meniul de alegere Host/Join e momentan text, in consola, ca sa nu
 * complicam cu randare de text in SDL de la inceput. Odata ce avem
 *2-3 jocuri gata, facem un meniu grafic cu iconite peste asta.
 *
 * Compilare / testare: vezi pc/CMakeLists.txt.
 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "net_pc.h"
#include "../common/net_proto.h"
#include "../games/pong.h"

#define WIN_W 640
#define WIN_H 480

static void draw_rect_norm(SDL_Renderer *r, float x, float y, float w, float h)
{
    SDL_Rect rect;
    rect.x = (int)(x * WIN_W);
    rect.y = (int)(y * WIN_H);
    rect.w = (int)(w * WIN_W);
    rect.h = (int)(h * WIN_H);
    SDL_RenderFillRect(r, &rect);
}

static void render_pong(SDL_Renderer *r, const PongState *s)
{
    SDL_SetRenderDrawColor(r, 10, 10, 20, 255);
    SDL_RenderClear(r);
    SDL_SetRenderDrawColor(r, 230, 230, 230, 255);

    draw_rect_norm(r, 0.01f, s->paddle_left_y, 0.02f, PONG_PADDLE_HEIGHT);
    draw_rect_norm(r, 0.97f, s->paddle_right_y, 0.02f, PONG_PADDLE_HEIGHT);
    draw_rect_norm(r, s->ball_x - 0.01f, s->ball_y - 0.01f, 0.02f, 0.02f);

    SDL_RenderPresent(r);
}

int main(int argc, char **argv)
{
    int is_host;
    char peer_ip[64] = {0};

    printf("=== PS2/PC Pong ===\n");
    printf("Gazduiesti (h) sau te conectezi (j)? ");
    char choice[8];
    if (!fgets(choice, sizeof(choice), stdin)) return 1;
    is_host = (choice[0] == 'h' || choice[0] == 'H');

    if (!is_host) {
        printf("IP-ul gazdei (host): ");
        if (!fgets(peer_ip, sizeof(peer_ip), stdin)) return 1;
        peer_ip[strcspn(peer_ip, "\r\n")] = 0;
    }

    int sock = net_pc_open(HUB_NET_PORT);
    if (sock < 0) {
        fprintf(stderr, "Nu am putut deschide socket-ul UDP\n");
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *win = SDL_CreateWindow("Pong PS2/PC", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    PongState state;
    pong_init(&state);

    uint32_t seq = 0;
    char client_ip[64] = {0}; /* completat de host cand primeste primul pachet */

    int running = 1;
    uint32_t last_ticks = SDL_GetTicks();

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = 0;
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        int my_input = 0;
        if (keys[SDL_SCANCODE_UP])   my_input = -1;
        if (keys[SDL_SCANCODE_DOWN]) my_input = 1;

        uint32_t now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;
        if (dt > 0.05f) dt = 0.05f; /* clamp, evita salturi mari daca am avut un lag */

        static int client_input = 0; /* ultimul input primit de la client */

        if (is_host) {
            /* citim input-ul clientului, daca a ajuns ceva; retinem si IP-ul lui */
            InputPacket in;
            int n;
            char sender_ip[64];
            while ((n = net_pc_recv_from(sock, &in, sizeof(in), sender_ip, sizeof(sender_ip))) > 0) {
                if (n == sizeof(in)) {
                    client_input = in.input_y;
                    if (strcmp(client_ip, sender_ip) != 0)
                        strncpy(client_ip, sender_ip, sizeof(client_ip) - 1);
                }
            }

            pong_update(&state, dt, my_input, client_input);

            StatePacket out;
            out.seq = seq++;
            out.ball_x = state.ball_x; out.ball_y = state.ball_y;
            out.ball_vx = state.ball_vx; out.ball_vy = state.ball_vy;
            out.paddle_host_y = state.paddle_left_y;
            out.paddle_client_y = state.paddle_right_y;
            out.score_host = state.score_left;
            out.score_client = state.score_right;

            if (client_ip[0])
                net_pc_send(sock, client_ip, HUB_NET_PORT, &out, sizeof(out));

        } else {
            InputPacket out;
            out.seq = seq++;
            out.input_y = (int8_t)my_input;
            net_pc_send(sock, peer_ip, HUB_NET_PORT, &out, sizeof(out));

            StatePacket in;
            int n;
            while ((n = net_pc_recv(sock, &in, sizeof(in))) > 0) {
                if (n == sizeof(in)) {
                    state.ball_x = in.ball_x; state.ball_y = in.ball_y;
                    state.paddle_left_y = in.paddle_host_y;
                    state.paddle_right_y = in.paddle_client_y;
                    state.score_left = in.score_host;
                    state.score_right = in.score_client;
                }
            }
        }

        render_pong(ren, &state);
        SDL_Delay(16);
    }

    net_pc_close(sock);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
