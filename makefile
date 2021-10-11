CC = gcc
CFLAGS = -Wall

PROJECTS = p0 p2 p3 p4

# p0
LIBOBJS_p0 = queue.o
OBJS_p0 = testafila.o
EXEC_p0 = testafila

# p2
LIBOBJS_p2 = ppos_core.o
HEADERS_p2 = ppos.h ppos_data.h
OBJS_p2_1 = pingpong-tasks1.o
EXEC_p2_1 = pingpong-tasks1
OBJS_p2_2 = pingpong-tasks2.o
EXEC_p2_2 = pingpong-tasks2
OBJS_p2_3 = pingpong-tasks3.o
EXEC_p2_3 = pingpong-tasks3

# p3
OBJS_p3 = pingpong-dispatcher.o
EXEC_p3 = pingpong-dispatcher

# p4
OBJS_p4 = pingpong-scheduler.o
EXEC_p4 = pingpong-scheduler


all: $(PROJECTS)

debug: clean flags $(PROJECTS)
flags:
	$(eval CFLAGS += -DDEBUG)

p0: $(EXEC_p0)
p2: $(EXEC_p2_1) $(EXEC_p2_2) $(EXEC_p2_3)
p3: $(EXEC_p3)
p4: $(EXEC_p4)

# p0
$(EXEC_p0): $(LIBOBJS_p0) $(OBJS_p0)
$(LIBOBJS_p0) : %.o : %.c %.h
$(OBJS_p0) : %.o : %.c

# p2
$(EXEC_p2_1): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p2_1)
$(LIBOBJS_p2) : %.o : %.c $(HEADERS_p2)
$(OBJS_p2_1) : %.o : %.c

$(EXEC_p2_2): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p2_2)
$(OBJS_p2_2) : %.o : %.c

$(EXEC_p2_3): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p2_3)
$(OBJS_p2_3) : %.o : %.c

# p3
$(EXEC_p3): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p3)
$(OBJS_p3) : %.o : %.c

# p4
$(EXEC_p4): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p4)
$(OBJS_p4) : %.o : %.c


clean:
	rm -f *.o

purge: clean
	rm -f $(EXEC_p0) $(EXEC_p2_1) $(EXEC_p2_2) $(EXEC_p2_3) $(EXEC_p3) *.out