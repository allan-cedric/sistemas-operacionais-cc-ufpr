CC = gcc
CFLAGS = -Wall

PROJECTS = p0 p1

# p0
LIBOBJS_p0 = queue.o
OBJS_p0 = testafila.o
EXEC_p0 = testafila

# p1
LIBOBJS_p1 = ppos_core.o
HEADERS_p1 = ppos.h ppos_data.h
OBJS_p1_1 = pingpong-tasks1.o
EXEC_p1_1 = pingpong-tasks1
OBJS_p1_2 = pingpong-tasks2.o
EXEC_p1_2 = pingpong-tasks2
OBJS_p1_3 = pingpong-tasks3.o
EXEC_p1_3 = pingpong-tasks3


all: $(PROJECTS)

debug: clean flags $(PROJECTS)
flags:
	$(eval CFLAGS += -DDEBUG)

p0: $(EXEC_p0)
p1: $(EXEC_p1_1) $(EXEC_p1_2) $(EXEC_p1_3)

# p0
$(EXEC_p0): $(LIBOBJS_p0) $(OBJS_p0)
$(LIBOBJS_p0) : %.o : %.c %.h
$(OBJS_p0) : %.o : %.c

# p1
$(EXEC_p1_1): $(LIBOBJS_p0) $(LIBOBJS_p1) $(OBJS_p1_1)
$(LIBOBJS_p1) : %.o : %.c $(HEADERS_p1)
$(OBJS_p1_1) : %.o : %.c

$(EXEC_p1_2): $(LIBOBJS_p0) $(LIBOBJS_p1) $(OBJS_p1_2)
$(OBJS_p1_2) : %.o : %.c

$(EXEC_p1_3): $(LIBOBJS_p0) $(LIBOBJS_p1) $(OBJS_p1_3)
$(OBJS_p1_3) : %.o : %.c

clean:
	rm -f *.o

purge: clean
	rm -f $(EXEC_p0) $(EXEC_p1_1) $(EXEC_p1_2) $(EXEC_p1_3) *.out