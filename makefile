CC = gcc
CFLAGS = -Wall

LIBOBJS = queue.o
OBJS = testafila.o
EXEC = testafila

all: $(EXEC)

$(EXEC): $(LIBOBJS) $(OBJS)

$(LIBOBJS) : %.o : %.c %.h
$(OBJS) : %.o : %.c

clean:
	rm -f *.o

purge: clean
	rm -f $(EXEC) *.out