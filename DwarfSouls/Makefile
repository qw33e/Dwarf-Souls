CC	= ../../../bin/lcc

all:
	$(CC) -o DwarfSouls.gb mini_souls.c map_map.c win_map.c map_tiles.c empty_tiles.c

compile.bat: Makefile
	@echo "REM Automatically generated from Makefile" > compile.bat
	@make -sn | sed y/\\//\\\\/ | grep -v make >> compile.bat

clean:
	rm -f *.o *.lst *.map *.gb *.ihx *.sym *.cdb *.adb *.asm
