CFLAGS=-lrt
rt_task:	rt_task.o dl_syscalls.o
	gcc $? -o $@ ${CFLAGS}
%.o:	%.c
	gcc -c $< -o $@
clean:
	rm -rf *.o rt_task
install:
	cp rt_task /usr/bin
uninstall:
	rm  /usr/bin/rt_task
