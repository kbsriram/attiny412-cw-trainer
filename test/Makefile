CC = gcc
CFLAGS = -g -Wall -I../src

test: key_test
	./key_test

key_test: key.o fake_hal_key.o key_test.o

key.o: ../src/key.c ../src/hal_key.h ../src/key.h
	$(CC) $(CFLAGS) -c ../src/key.c -o $@

fake_hal_key.o: fake_hal_key.c ../src/hal_key.h

key_test.o: ../src/key.h

clean:
	rm -f *.o key_test *~
