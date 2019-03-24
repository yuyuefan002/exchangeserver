CC=g++
CFLAGS=-std=gnu++11 -pedantic -Wall -Werror -ggdb3 -pthread
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
EXCHMatchServer:$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)
%.o:%.cpp
	$(CC) $(CFLAGS) -c $<
.PHONY: clean
clean:
	rm -f EXCHMatchServer *.o *~ a.out
