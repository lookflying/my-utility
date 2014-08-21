CFLAGS=-lrt
OBJ= rt_task.o dl_syscalls.o

rt_task: ${OBJ}
	gcc $? -o $@ ${CFLAGS}

${OBJ}:	%.o: %.c
	gcc -c $< -o $@
clean:
	rm -rf *.o rt_task
install:
	cp rt_task /usr/bin
uninstall:
	rm  /usr/bin/rt_task
