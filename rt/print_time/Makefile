CC=gcc

OBJ = print_time.o
print_time: ${OBJ}

${OBJ} : %.o: %.c
	${CC} -c ${CFLAGS} $< -o $@

clean:
	rm -rf *.o
