all: 1dbar.o 1dbar

1dbar: 1dbar.c AXL/axl.o 1dbar.h
	cc -O -g -o $@ $< -lpopt -IAXL -I. -D_GNU_SOURCE AXL/axl.o -lcurl --std=gnu99

1dbar.o: 1dbar.c AXL/axl.o 1dbar.h
	cc -g -Wall -Wextra -DLIB -O -c -o 1dbar.o 1dbar.c -I. -IAXL -D_GNU_SOURCE --std=gnu99


AXL/axl.o: AXL/axl.c
	make -C AXL

update:
	git submodule update --init
