OUT = libtuner.so
SRC = ${wildcard math/*.c}
OBJ = ${SRC:.c=.o}

CFLAGS:=-std=gnu99 -Iinclude \
	-Wall -Wextra -pedantic -fPIC \
	-Wl,-soname,libtuner.so.1 \
	${CFLAGS}

LDFLAGS:=-shared -lm ${LDFLAGS}

all: ${OUT}

${OUT}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm ${OUT} ${OBJ}

.PHONY: clean
