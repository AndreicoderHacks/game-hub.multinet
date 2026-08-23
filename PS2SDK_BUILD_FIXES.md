# PS2SDK build fixes

This copy was prepared for the current `ps2dev/ps2dev:latest` GitHub Actions environment.

Changes in `ps2/main.c`:
- `ps2ip_init()` -> `ps2ipInit()`
- removed the obsolete `struct ip_info`/legacy config usage
- removed the unavailable `gsKit_flip()` call
- retained the existing `gsKit_sync_flip()` path
- `inet_aton()` -> `inet_addr()` assignment
- added `<sys/ioctl.h>` for `ioctl()`

The existing Makefile was otherwise left intact to avoid changing the project's
IRX/link configuration without testing the actual GitHub runner.
