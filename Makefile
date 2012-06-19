OUT = libtuner.so
LIB = ${OUT}.1
SRC = ${wildcard math/*.c}
OBJ = ${SRC:.c=.o}

CFLAGS:=-std=gnu99 -Iinclude \
	-Wall -Wextra -pedantic -fPIC \
	${CFLAGS}

LDFLAGS:=-shared -lm \
	-Wl,-soname,${LIB} \
	${LDFLAGS}

all: ${OUT}

${OUT}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}
	ln -s ${OUT} ${LIB}

clean:
	rm ${OUT} ${OBJ} ${LIB}

.PHONY: clean
