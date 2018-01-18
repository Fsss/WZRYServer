CC	= g++
LD	= ld
AR	= ar
CFLAGS 	= -ggdb -Wall -Werror
STDFLAG	= -std=c++11

FILES	= $(shell find . -name "*.cpp")
OBJS	= $(FILES:.cpp=.o)

all	= $(OJBS)

server:$(OBJS)
	$(CC) $(CFLAGS) $(STDFLAG) -o sever
 
clean:
	rm *.o
	rm $(LIB_PATH)/libexnet.a

