CC = gcc
CFLAGS = -g -Wall -I../src

test: run_key_test run_morse_test

run_key_test: key_test
	./key_test

run_morse_test: morse_test
	./morse_test

key_test: key.o fake_hal_key.o key_test.o

morse_test: morse.o morse_test.o

key.o: ../src/key.c ../src/hal_key.h ../src/key.h
	$(CC) $(CFLAGS) -c ../src/key.c -o $@

morse.o: ../src/morse.c ../src/morse.h
	$(CC) $(CFLAGS) -c ../src/morse.c -o $@

fake_hal_key.o: fake_hal_key.c ../src/hal_key.h

key_test.o: ../src/key.h key_test.c

morse_test.o: ../src/morse.h morse_test.c

clean:
	rm -f *.o key_test *~
