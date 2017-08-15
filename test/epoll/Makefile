GCC=gcc
FLAG= -O2 -Iinclude -lstdc++


all: epoll_tutorial.o main.o tutorial

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

clean:
	rm *.o
