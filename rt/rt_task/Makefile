CFLAGS=-lrt
SRC= rt_task.c dl_syscalls.c
rt_task: ${SRC}
	gcc $? -o $@ ${CFLAGS}
clean:
	rm -rf *.o rt_task
install:
	cp rt_task /usr/bin
uninstall:
	rm  /usr/bin/rt_task
