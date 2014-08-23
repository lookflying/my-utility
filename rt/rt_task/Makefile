CFLAGS=-lrt
SRC=rt_task.c dl_syscalls.c
OBJ=rt_task.o dl_syscalls.o
rt_task: ${OBJ}
	gcc ${OBJ} -o $@ ${CFLAGS}

dl_syscalls.o:	dl_syscalls.h dl_syscalls.c
	gcc -c dl_syscalls.c -o $@
rt_task.o:	rt_task.h rt_task.c
	gcc -c rt_task.c -o $@
clean:
	rm -rf *.o rt_task
install:
	cp rt_task /usr/bin
uninstall:
	rm  /usr/bin/rt_task
