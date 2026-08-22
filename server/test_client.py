import socket
import struct
import sys
import time

if len(sys.argv) < 2:
    print("Foloseste: python test_client.py <ip_server>")
    sys.exit(1)

SERVER = (sys.argv[1], 9999)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setblocking(False)

INPUT_UP = 1  # trimite ca player 1: apasa "sus" continuu, doar ca sa vedem ca merge

print("Trimit input fals la server, Ctrl+C sa opresti...")

while True:
    packet = struct.pack("<BB", 0x02, INPUT_UP)
    sock.sendto(packet, SERVER)

    try:
        data, addr = sock.recvfrom(64)
        if len(data) >= 15 and data[0] == 0x10:
            _, p1_y, p2_y, ball_x, ball_y, s1, s2 = struct.unpack("<Bffff BB", data)
            print(f"p1_y={p1_y:.1f} p2_y={p2_y:.1f} ball=({ball_x:.1f},{ball_y:.1f}) scor={s1}-{s2}")
    except BlockingIOError:
        pass
    except ConnectionResetError:
        # Windows transforma un ICMP "port unreachable" (serverul nu asculta)
        # intr-o eroare falsa de conexiune -- pe UDP real nu exista conexiuni
        pass

    time.sleep(1 / 30)
