CC=gcc
CFLAGS=-lrt
OBJ = print_time.o
print_time: ${OBJ}
	${CC} $< -o $@ ${CFLAGS} 

${OBJ} : %.o: %.c
	${CC} -c $< -o $@ ${CFLAGS} 

clean:
	rm -rf *.o

install:
	cp print_time /usr/bin	
