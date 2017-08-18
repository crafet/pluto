GCC=gcc
FLAG= -O2 -Iinclude -lstdc++


all: epoll_tutorial.o main.o tutorial buffer_test.o buffer_test libev_test.o libev_test clean

## if exist file "all" in this directory
## then make all will do nothing

.PHONY: all

OBJ=epoll_tutorial.o main.o

epoll_tutorial.o:src/epoll_tutorial.cpp
	${GCC} ${FLAG} -c $< -o $@

## $< first prerequisite
## $@ the target
main.o:src/epoll_tutorial_test.cpp
	${GCC} ${FLAG} -c $< -o $@

tutorial:${OBJ}
	${GCC} -o $@ ${OBJ} -lstdc++

buffer_test.o:src/buffer_test.cpp
	${GCC} ${FLAG} -c $< -o $@

buffer_test:buffer_test.o
	${GCC} -o $@ buffer_test.o -lstdc++


libev_test.o:src/libev_test.cpp
	${GCC} ${FLAG} -c $< -o $@
libev_test:libev_test.o
	${GCC} -o $@ $< -lstdc++ -lev
clean:
	rm *.o
