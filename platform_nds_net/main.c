#include <nds.h>
#include <dswifi9.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SCREEN_W 256
#define SCREEN_H 192
#define PADDLE_W 8
#define PADDLE_H 40
#define BALL_SIZE 6

/* SCHIMBA cu IP-ul laptopului pe reteaua locala */
#define SERVER_IP   "192.168.1.8"
#define SERVER_PORT 9999

#define INPUT_UP   1
#define INPUT_DOWN 2

static u16 *fb;
static int sock;
static struct sockaddr_in server_addr;

static inline u16 rgb15(u8 r, u8 g, u8 b)
{
    return RGB15(r >> 3, g >> 3, b >> 3) | BIT(15);
}

static void clear_screen(void)
{
    dmaFillHalfWords(0, fb, SCREEN_W * SCREEN_H * 2);
}

static void draw_rect(int x, int y, int w, int h, u8 r, u8 g, u8 b)
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

static bool wifi_connect(void)
{
    /* Foloseste conexiunea salvata in setarile firmware ale DS-ului
       (System Settings -> Internet -> alege reteaua ta open) */
    if (!Wifi_InitDefault(WFC_CONNECT)) {
        return false;
    }
    return true;
}

int main(void)
{
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    fb = bgGetGfxPtr(bg);

    consoleDemoInit();
    iprintf("Conectare WiFi...\n");

    if (!wifi_connect()) {
        iprintf("Conexiune WiFi esuata.\n");
        while (1) swiWaitForVBlank();
    }
    iprintf("WiFi OK. Deschid socket...\n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_aton(SERVER_IP, &server_addr.sin_addr);

    /* socket non-blocant, ca sa nu ne inghete bucla de joc asteptand pachete */
    int flags = 1;
    ioctl(sock, FIONBIO, &flags);

    float p1_y = 0, p2_y = 0, ball_x = 0, ball_y = 0;
    int score1 = 0, score2 = 0;

    while (1) {
        scanKeys();
        int keys = keysHeld();
        u8 input = 0;
        if (keys & KEY_UP)   input |= INPUT_UP;
        if (keys & KEY_DOWN) input |= INPUT_DOWN;

        u8 out[2] = { 0x02, input };
        sendto(sock, out, 2, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        u8 in[64];
        int n = recvfrom(sock, in, sizeof(in), 0, NULL, NULL);
        if (n >= 19 && in[0] == 0x10) {
            memcpy(&p1_y, in + 1, 4);
            memcpy(&p2_y, in + 5, 4);
            memcpy(&ball_x, in + 9, 4);
            memcpy(&ball_y, in + 13, 4);
            score1 = in[17];
            score2 = in[18];
        }

        clear_screen();
        draw_rect(0, (int)p1_y, PADDLE_W, PADDLE_H, 255, 255, 255);
        draw_rect(SCREEN_W - PADDLE_W, (int)p2_y, PADDLE_W, PADDLE_H, 255, 255, 255);
        draw_rect((int)ball_x, (int)ball_y, BALL_SIZE, BALL_SIZE, 255, 255, 255);

        swiWaitForVBlank();
    }

    return 0;
}
