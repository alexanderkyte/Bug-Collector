
install: liballoc.so
	sudo cp liballoc.so /lib

liballoc.so: alloc.dats
	patscc -c -fPIC alloc.dats
	gcc alloc_dats.o -shared -o liballoc.so

clean:
	rm alloc_dats.c
	rm alloc_dats.o
	rm liballoc.so
