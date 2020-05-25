all: 1dbar

1dbar: 1dbar.c AXL/axl.o
	cc -O -o $@ $< -lpopt -IAXL -I. -D_GNU_SOURCE AXL/axl.o -lcurl

AXL/axl.o: AXL/axl.c
	make -C AXL

update:
	git submodule update --init
