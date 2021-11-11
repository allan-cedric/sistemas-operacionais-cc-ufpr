CC = gcc
CFLAGS = -Wall
LDLIBS = -lm

PROJECTS = p0 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12

# p0
LIBOBJS_p0 = queue.o
OBJS_p0 = testafila.o
EXEC_p0 = testafila

# p2
LIBOBJS_p2 = ppos_core.o ppos_ipc.o
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

# p5
OBJS_p5_1 = pingpong-preempcao.o
EXEC_p5_1 = pingpong-preempcao
OBJS_p5_2 = pingpong-preempcao-stress.o
EXEC_p5_2 = pingpong-preempcao-stress

# p6
OBJS_p6_1 = pingpong-contab.o
EXEC_p6_1 = pingpong-contab
OBJS_p6_2 = pingpong-contab-prio.o
EXEC_p6_2 = pingpong-contab-prio

# p7
OBJS_p7 = pingpong-maintask.o
EXEC_p7 = pingpong-maintask

# p8
OBJS_p8 = pingpong-join.o
EXEC_p8 = pingpong-join

# p9
OBJS_p9 = pingpong-sleep.o
EXEC_p9 = pingpong-sleep

# p10
OBJS_p10_1 = pingpong-semaphore.o
EXEC_p10_1 = pingpong-semaphore
OBJS_p10_2 = pingpong-racecond.o
EXEC_p10_2 = pingpong-racecond

# p11
OBJS_p11 = pingpong-prodcons.o
EXEC_p11 = pingpong-prodcons

# p12
OBJS_p12 = pingpong-mqueue.o
EXEC_p12 = pingpong-mqueue

all: $(PROJECTS)

debug: clean flags $(PROJECTS)
flags:
	$(eval CFLAGS += -DDEBUG)

p0: $(EXEC_p0)
p2: $(EXEC_p2_1) $(EXEC_p2_2) $(EXEC_p2_3)
p3: $(EXEC_p3)
p4: $(EXEC_p4)
p5: $(EXEC_p5_1) $(EXEC_p5_2)
p6: $(EXEC_p6_1) $(EXEC_p6_2)
p7: $(EXEC_p7)
p8: $(EXEC_p8)
p9: $(EXEC_p9)
p10: $(EXEC_p10_1) $(EXEC_p10_2)
p11: $(EXEC_p11)
p12: $(EXEC_p12)

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

# p5
$(EXEC_p5_1): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p5_1)
$(OBJS_p5_1) : %.o : %.c

$(EXEC_p5_2): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p5_2)
$(OBJS_p5_2) : %.o : %.c

# p6
$(EXEC_p6_1): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p6_1)
$(OBJS_p6_1) : %.o : %.c

$(EXEC_p6_2): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p6_2)
$(OBJS_p6_2) : %.o : %.c

# p7
$(EXEC_p7): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p7)
$(OBJS_p7) : %.o : %.c

# p8
$(EXEC_p8): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p8)
$(OBJS_p8) : %.o : %.c

# p9
$(EXEC_p9): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p9)
$(OBJS_p9) : %.o : %.c

# p10
$(EXEC_p10_1): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p10_1)
$(OBJS_p10_1) : %.o : %.c

$(EXEC_p10_2): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p10_2)
$(OBJS_p10_2) : %.o : %.c

# p11
$(EXEC_p11): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p11)
$(OBJS_p11) : %.o : %.c

# p12
$(EXEC_p12): $(LIBOBJS_p0) $(LIBOBJS_p2) $(OBJS_p12)
$(OBJS_p12) : %.o : %.c

clean:
	rm -f *.o

purge: clean
	rm -f $(EXEC_p0) $(EXEC_p2_1) $(EXEC_p2_2) $(EXEC_p2_3) \
	$(EXEC_p3) $(EXEC_p4) $(EXEC_p5_1) $(EXEC_p5_2) $(EXEC_p6_1) \
	$(EXEC_p6_2) $(EXEC_p7) $(EXEC_p8) $(EXEC_p9) $(EXEC_p10_1) $(EXEC_p10_2) \
	$(EXEC_p11) $(EXEC_p12) *.out