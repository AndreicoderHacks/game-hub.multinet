# game-hub

Hub multiplatformă de minijocuri 2D (Pong primul, apoi Flappy Bird etc).

## Structură
- `core/` — logica jocurilor, C portabil, nu știe pe ce platformă rulează
- `platform_web/` — backend SDL2 + Emscripten → un singur build web care merge pe telefon, Windows, Linux (orice browser)
- `platform_nds/` — backend libnds pentru DS Lite (nu poate intra pe browser, cod nativ mic)
- `.github/workflows/build.yml` — CI: compilează ambele la fiecare push, artefacte descărcabile din tab-ul Actions

## Workflow
1. Push pe `main`
2. GitHub Actions compilează `game-hub-web` (deschizi `index.html` în browser, pe telefon/laptop) și `game-hub-nds` (`.nds`, pui pe cardul DS-ului modat)
3. Descarci artefactele din Actions → Summary → Artifacts, testezi, îmi trimiți erorile
