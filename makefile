CC = gcc
CFLAGS = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
CLINKINGFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

.PHONY: clean

memory_tests.o: memory_tests.c

nand.o: nand.c

llist.o: llist.c

libnand.so: nand.o llist.o memory_tests.o
	$(CC) $(CLINKINGFLAGS) -o $@ $^
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./

testy.o: testy.c

tester: testy.o libnand.so
	gcc -L. -o $@ $< -lnand

nand_example.o: nand_example.c

nand_example: nand_example.o libnand.so
	gcc -L. -o $@ $< -lnand

TEST_RELATED= test.in tester auto test_gen nand_example
clean:
	rm -rf *.o *.out libnand.so $(TEST_RELATED)