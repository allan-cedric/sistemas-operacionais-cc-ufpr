CC = gcc
CFLAGS = -Wall

OBJS = queue.o testafila.o
EXEC = testafila

all: $(EXEC)

$(EXEC): $(OBJS)

$(OBJS) : %.o : %.c

clean:
	rm -f *.o

purge: clean
	rm -f $(EXEC)