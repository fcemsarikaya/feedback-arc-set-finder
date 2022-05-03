CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

GENERATOR_OBJECTS = generator.o circular_buffer.o
SUPERVISOR_OBJECTS = supervisor.o circular_buffer.o

.PHONY: all clean
all: generator supervisor


generator: $(GENERATOR_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lrt -pthread

supervisor: $(SUPERVISOR_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lrt -pthread

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


generator.o: generator.c generator.h circular_buffer.h supervisor.h
circular_buffer.o: circular_buffer.c circular_buffer.h generator.h
supervisor.o: supervisor.c supervisor.h generator.h circular_buffer.h

clean:
	rm -rf *.o all
