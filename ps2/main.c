/* Hub PS2 (gsKit) - deocamdata doar Pong, ca si partea de PC.
 *
 * ATENTIE: partea de init retea (blocul "network init" de mai jos) e
 * cea mai fragila bucata din tot proiectul - IRX-urile trebuie sa se
 * incarce in ordinea corecta si depinde de placa de retea (SMAP,
 * built-in pe PS2 slim). Daca pica exact aici la primul build, e
 * normal, iteram impreuna pe log-ul de eroare.
 */

#include <kernel.h>
#include <sifrpc.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <loadfile.h>
#include <sbv_patches.h>

#include <ps2ip.h>
#include <netman.h>

#include <gsKit.h>
#include <dmaKit.h>
#include <libpad.h>

#include <string.h>
#include <stdio.h>

#include "../common/net_proto.h"
#include "../games/pong.h"

/* IRX-urile astea sunt convertite in .o de catre Makefile (vezi regula bin2o)
 * si linkuite direct in ELF - nu trebuie fisiere separate pe memory card. */
extern u8 netman_irx[];
extern int size_netman_irx;
extern u8 smap_irx[];
extern int size_smap_irx;
extern u8 ps2ip_irx[];
extern int size_ps2ip_irx;

static GSGLOBAL *gsGlobal;

static char pad_buf[256] __attribute__((aligned(64)));

/* --- network init ------------------------------------------------ */

static int net_ready = 0;

static void network_init(void)
{
    SifInitRpc(0);

    SifExecModuleBuffer(netman_irx, size_netman_irx, 0, NULL, NULL);
    SifExecModuleBuffer(smap_irx, size_smap_irx, 0, NULL, NULL);
    SifExecModuleBuffer(ps2ip_irx, size_ps2ip_irx, 0, NULL, NULL);

    ps2ip_init();

    /* DHCP - asteptam sa primim IP de la router.
       Daca routerul nu are DHCP, aici trebuie IP static (vezi ps2ip docs). */
    int retries = 0;
    struct ip_info ipinfo;
    while (retries < 300) { /* ~10s la 30fps de asteptare */
        if (ps2ip_getconfig("sm0", &ipinfo) >= 0 && ipinfo.ipaddr.addr != 0) {
            net_ready = 1;
            break;
        }
        retries++;
    }
}

/* --- input --------------------------------------------------------- */

static int read_pad_y(void)
{
    struct padButtonStatus buttons;
    if (padGetState(0, 0) == PAD_STATE_STABLE) {
        padRead(0, 0, &buttons);
        if (buttons.btns & PAD_UP)   return -1;
        if (buttons.btns & PAD_DOWN) return 1;
    }
    return 0;
}

/* --- rendering ------------------------------------------------------ */

static void render_pong(const PongState *s)
{
    gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x10, 0x10, 0x20, 0x00, 0x00));

    int w = gsGlobal->Width, h = gsGlobal->Height;

    gsKit_prim_sprite(gsGlobal,
        w * 0.01f, h * s->paddle_left_y,
        w * 0.03f, h * (s->paddle_left_y + PONG_PADDLE_HEIGHT),
        1, GS_SETREG_RGBAQ(0xE0, 0xE0, 0xE0, 0x80, 0x00));

    gsKit_prim_sprite(gsGlobal,
        w * 0.97f, h * s->paddle_right_y,
        w * 0.99f, h * (s->paddle_right_y + PONG_PADDLE_HEIGHT),
        1, GS_SETREG_RGBAQ(0xE0, 0xE0, 0xE0, 0x80, 0x00));

    gsKit_prim_sprite(gsGlobal,
        w * (s->ball_x - 0.01f), h * (s->ball_y - 0.01f),
        w * (s->ball_x + 0.01f), h * (s->ball_y + 0.01f),
        1, GS_SETREG_RGBAQ(0xE0, 0xE0, 0xE0, 0x80, 0x00));

    gsKit_queue_exec(gsGlobal);
    gsKit_finish();
    gsKit_flip(gsGlobal);
    gsKit_sync_flip(gsGlobal);
}

int main(int argc, char **argv)
{
    /* --- init grafica --- */
    gsGlobal = gsKit_init_global();
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
        D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);
    gsKit_init_screen(gsGlobal);

    /* --- init pad --- */
    padInit(0);
    padPortOpen(0, 0, pad_buf);

    /* --- init retea --- */
    network_init();
    int sock = -1;
    if (net_ready) {
        struct sockaddr_in addr;
        sock = lwip_socket(AF_INET, SOCK_DGRAM, 0);
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(HUB_NET_PORT);
        lwip_bind(sock, (struct sockaddr *)&addr, sizeof(addr));
        /* non-blocking */
        int flags = 1;
        lwip_ioctl(sock, FIONBIO, &flags);
    }

    /* TODO: hostul si IP-ul tinta - deocamdata harcodat pentru primul test.
       Schimba HOST_MODE si PEER_IP dupa cum testezi (host pe PC sau pe PS2). */
    #define HOST_MODE 0        /* 0 = PS2 e client, 1 = PS2 e host */
    static char peer_ip[] = "192.168.1.100"; /* <-- pune IP-ul real al PC-ului aici */

    PongState state;
    pong_init(&state);
    uint32_t seq = 0;

    while (1) {
        int my_input = read_pad_y();

        if (net_ready && sock >= 0) {
            if (HOST_MODE) {
                InputPacket in;
                struct sockaddr_in from;
                socklen_t fromlen = sizeof(from);
                int n = lwip_recvfrom(sock, &in, sizeof(in), 0,
                    (struct sockaddr *)&from, &fromlen);

                int client_input = (n == sizeof(in)) ? in.input_y : 0;
                pong_update(&state, 1.0f / 60.0f, my_input, client_input);

                StatePacket out;
                out.seq = seq++;
                out.ball_x = state.ball_x; out.ball_y = state.ball_y;
                out.paddle_host_y = state.paddle_left_y;
                out.paddle_client_y = state.paddle_right_y;
                out.score_host = state.score_left;
                out.score_client = state.score_right;
                lwip_sendto(sock, &out, sizeof(out), 0,
                    (struct sockaddr *)&from, sizeof(from));
            } else {
                InputPacket out;
                out.seq = seq++;
                out.input_y = (int8_t)my_input;

                struct sockaddr_in dst;
                memset(&dst, 0, sizeof(dst));
                dst.sin_family = AF_INET;
                dst.sin_port = htons(HUB_NET_PORT);
                inet_aton(peer_ip, &dst.sin_addr);
                lwip_sendto(sock, &out, sizeof(out), 0,
                    (struct sockaddr *)&dst, sizeof(dst));

                StatePacket in;
                int n = lwip_recvfrom(sock, &in, sizeof(in), 0, NULL, NULL);
                if (n == sizeof(in)) {
                    state.ball_x = in.ball_x; state.ball_y = in.ball_y;
                    state.paddle_left_y = in.paddle_host_y;
                    state.paddle_right_y = in.paddle_client_y;
                    state.score_left = in.score_host;
                    state.score_right = in.score_client;
                }
            }
        } else {
            /* fara retea (inca) - macar rulam local ca sa vedem ca merge randarea */
            pong_update(&state, 1.0f / 60.0f, my_input, 0);
        }

        render_pong(&state);
    }

    return 0;
}
