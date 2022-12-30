OBJS	= *.o
SOURCE	= *.cpp
HEADER	= *.hpp
OUT	= mdb.out
CC	 = g++
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) -lelf -lcapstone

*.o: *.cpp
	$(CC) $(FLAGS) *.cpp 


clean:
	rm -f $(OBJS) $(OUT)

run: $(OUT)
	./$(OUT)
debug: $(OUT)
	$(CC) -g $(OBJS) -DDEBUG -o $(OUT) $(LFLAGS)

valgrind: $(OUT)
	valgrind $(OUT)

valgrind_leakcheck: $(OUT)
	valgrind --leak-check=full $(OUT)

valgrind_extreme: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes $(OUT)