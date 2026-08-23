# PS2 / PC minigame hub

Primul joc: Pong, multiplayer local (PS2 slim <-> PC) prin UDP pe rețeaua locală.

## Structură

- `common/net_proto.h` — protocolul UDP comun (structuri de pachete)
- `games/pong.c` / `pong.h` — logica jocului, identică pe ambele platforme
- `pc/` — frontend SDL2 pentru PC (build cu CMake)
- `ps2/` — frontend gsKit pentru PS2 (build cu PS2SDK / Makefile)
- `.github/workflows/build.yml` — build automat pentru ambele ținte

## Cum funcționează rețeaua

Model host-client simplu:
- **Hostul** rulează simularea completă (poziție minge, coliziuni, scor) și
  trimite starea jocului către client, la fiecare frame.
- **Clientul** trimite doar inputul lui (sus/jos) și primește starea de la host.

Asta înseamnă mai puțin de sincronizat și hostul are ultimul cuvânt — nu trebuie
reconciliere de stări diferite între cele două părți.

## De testat/de rulat

**PC:** după build, rulează `pong_pc`, alege `h` (host) sau `j` (join, apoi
introdu IP-ul hostului).

**PS2:** ATENȚIE — IP-ul țintă (`peer_ip`) și modul (`HOST_MODE`) sunt
deocamdată hardcodate în `ps2/main.c`, la începutul lui `main()`. Editează-le
înainte de build în funcție de ce testezi (PS2 ca și client către PC, sau
invers).

## Ce urmează

1. Primul test real: PC ca host, PS2 ca client (sau invers), pe același LAN.
2. Partea cea mai probabil să dea probleme la primul build/test e inițializarea
   de rețea pe PS2 (`network_init()` din `ps2/main.c`) — dacă pică aici,
   trimite-mi log-ul de eroare din GitHub Actions / de pe consolă.
3. După ce merge Pong, adăugăm jocul 2 (stil Doodle Jump) și 3 (shooter pixel),
   refolosind protocolul de rețea și structura de hub.
4. Meniu grafic (hub cu iconițe) în loc de promptul text actual — după ce avem
   toate cele 3 jocuri gata.
