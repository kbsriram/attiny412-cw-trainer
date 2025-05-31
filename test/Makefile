CC = gcc
CFLAGS = -g -Wall -I../src

test: run_key_test run_morse_test run_state_test run_capture_test

run_key_test: key_test
	./key_test

run_morse_test: morse_test
	./morse_test

run_state_test: state_test
	./state_test

run_capture_test: capture_test
	./capture_test

key_test: key.o fake_hal_key.o key_test.o

morse_test: morse.o morse_test.o

state_test: state.o state_test.o fake_tone.o  morse.o capture.o key.o fake_hal_key.o

capture_test: capture.o capture_test.o morse.o capture.o

key.o: ../src/key.c ../src/hal_key.h ../src/key.h
	$(CC) $(CFLAGS) -c ../src/key.c -o $@

morse.o: ../src/morse.c ../src/morse.h
	$(CC) $(CFLAGS) -c ../src/morse.c -o $@

capture.o: ../src/capture.c ../src/capture.h ../src/morse.h
	$(CC) $(CFLAGS) -c ../src/capture.c -o $@

state.o: ../src/state.c ../src/morse.h ../src/state.h
	$(CC) $(CFLAGS) -c ../src/state.c -o $@

fake_hal_key.o: fake_hal_key.c ../src/hal_key.h

fake_tone.o: fake_tone.c ../src/tone.h

key_test.o: ../src/key.h key_test.c

morse_test.o: ../src/morse.h morse_test.c

capture_test.o: ../src/capture.h ../src/morse.h capture_test.c

clean:
	rm -f *.o key_test *~
