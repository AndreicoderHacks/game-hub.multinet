#ifndef NET_PROTO_H
#define NET_PROTO_H

#include <stdint.h>

/* Portul UDP folosit pentru toate jocurile din hub.
 * Host-ul (cine găzduiește meciul) ascultă pe portul ăsta.
 */
#define HUB_NET_PORT 9999

/* Ce trimite CLIENTUL către HOST, la fiecare frame:
 * doar inputul lui, nimic altceva. Hostul e autoritatea
 * pentru starea jocului -> mai puțin de sincronizat, mai
 * greu de "trișat", mai simplu de scris.
 */
typedef struct {
    uint32_t seq;      /* numar de secventa, ca sa aruncam pachete vechi/duplicate */
    int8_t   input_y;  /* -1 = sus, 0 = neutru, 1 = jos (paddle) */
} __attribute__((packed)) InputPacket;

/* Ce trimite HOSTUL către CLIENT, la fiecare frame:
 * starea completă a jocului de Pong.
 */
typedef struct {
    uint32_t seq;
    float    ball_x, ball_y;
    float    ball_vx, ball_vy;
    float    paddle_host_y;
    float    paddle_client_y;
    uint8_t  score_host;
    uint8_t  score_client;
} __attribute__((packed)) StatePacket;

#endif /* NET_PROTO_H */
