EE_BIN = pong_ps2.elf
EE_OBJS = main.o pong.o \
          netman_irx.o smap_irx.o ps2ip_irx.o

EE_LIBS = -lgskit -ldmakit -lpad -lps2ip -lnetman -lc

EE_INCS = -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include -I..

all: $(EE_BIN)

pong.o: ../games/pong.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c ../games/pong.c -o pong.o

# convertim IRX-urile din PS2SDK in obiecte .o linkuibile
netman_irx.o: $(PS2SDK)/iop/irx/netman.irx
	$(EE_BIN2C) $< netman_irx.c netman_irx
	$(EE_CC) $(EE_CFLAGS) -c netman_irx.c -o $@

smap_irx.o: $(PS2SDK)/iop/irx/smap.irx
	$(EE_BIN2C) $< smap_irx.c smap_irx
	$(EE_CC) $(EE_CFLAGS) -c smap_irx.c -o $@

ps2ip_irx.o: $(PS2SDK)/iop/irx/ps2ip.irx
	$(EE_BIN2C) $< ps2ip_irx.c ps2ip_irx
	$(EE_CC) $(EE_CFLAGS) -c ps2ip_irx.c -o $@

clean:
	rm -f $(EE_BIN) *.o *_irx.c

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
