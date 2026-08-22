import socket
import struct
import time

HOST = "0.0.0.0"
PORT = 9999

SCREEN_W, SCREEN_H = 256, 192
PADDLE_W, PADDLE_H = 8, 40
BALL_SIZE = 6
PADDLE_SPEED = 160.0
BALL_SPEED = 140.0

INPUT_P1_UP, INPUT_P1_DOWN = 1, 2
INPUT_P2_UP, INPUT_P2_DOWN = 4, 8

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))
sock.setblocking(False)

players = {}  # addr -> player index (0 or 1)
inputs = [0, 0]

p1_y = p2_y = SCREEN_H / 2 - PADDLE_H / 2
ball_x = SCREEN_W / 2 - BALL_SIZE / 2
ball_y = SCREEN_H / 2 - BALL_SIZE / 2
ball_vx, ball_vy = BALL_SPEED, BALL_SPEED * 0.5
score1 = score2 = 0


def reset_ball(direction):
    global ball_x, ball_y, ball_vx, ball_vy
    ball_x = SCREEN_W / 2 - BALL_SIZE / 2
    ball_y = SCREEN_H / 2 - BALL_SIZE / 2
    ball_vx = BALL_SPEED * direction
    ball_vy = BALL_SPEED * 0.5


def handle_packet(data, addr):
    global players

    # ignora orice pachet care nu e exact formatul nostru de input
    # (tip 0x02 + 1 byte input) -- opreste zgomotul de retea sa fie
    # confundat cu un jucator nou
    if len(data) != 2 or data[0] != 0x02:
        return

    if addr not in players:
        if len(players) >= 2:
            return  # meci plin
        players[addr] = len(players)
        print(f"Player {players[addr]} conectat de la {addr}")

    pid = players[addr]
    inputs[pid] = data[1]


def update(dt):
    global p1_y, p2_y, ball_x, ball_y, ball_vx, ball_vy, score1, score2

    if inputs[0] & INPUT_P1_UP:   p1_y -= PADDLE_SPEED * dt
    if inputs[0] & INPUT_P1_DOWN: p1_y += PADDLE_SPEED * dt
    if inputs[1] & INPUT_P2_UP:   p2_y -= PADDLE_SPEED * dt
    if inputs[1] & INPUT_P2_DOWN: p2_y += PADDLE_SPEED * dt

    p1_y = max(0, min(SCREEN_H - PADDLE_H, p1_y))
    p2_y = max(0, min(SCREEN_H - PADDLE_H, p2_y))

    ball_x += ball_vx * dt
    ball_y += ball_vy * dt

    if ball_y < 0 or ball_y > SCREEN_H - BALL_SIZE:
        ball_vy = -ball_vy

    if ball_x < PADDLE_W and p1_y < ball_y + BALL_SIZE and ball_y < p1_y + PADDLE_H:
        ball_vx = -ball_vx
        ball_x = PADDLE_W

    if ball_x > SCREEN_W - PADDLE_W - BALL_SIZE and p2_y < ball_y + BALL_SIZE and ball_y < p2_y + PADDLE_H:
        ball_vx = -ball_vx
        ball_x = SCREEN_W - PADDLE_W - BALL_SIZE

    if ball_x < 0:
        score2 += 1
        reset_ball(1)
    if ball_x > SCREEN_W:
        score1 += 1
        reset_ball(-1)


def broadcast_state():
    # tip 0x10 + 4 floats (p1_y, p2_y, ball_x, ball_y) + 2 bytes scor
    payload = struct.pack("<Bffff BB", 0x10, p1_y, p2_y, ball_x, ball_y, score1 & 0xFF, score2 & 0xFF)
    for addr in players:
        sock.sendto(payload, addr)


print(f"Server Pong pe {HOST}:{PORT}, astept jucatori...")
last = time.time()
TICK = 1.0 / 30.0

while True:
    now = time.time()
    dt = now - last

    if dt >= TICK:
        while True:
            try:
                data, addr = sock.recvfrom(64)
                handle_packet(data, addr)
            except BlockingIOError:
                break

        update(dt)
        broadcast_state()
        last = now
    else:
        time.sleep(TICK - dt)
