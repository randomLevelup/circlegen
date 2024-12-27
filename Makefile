CC = g++
CFLAGS = -Wall -Wextra -Wpedantic -I./include -I/usr/local/include/ -I/usr/include/eigen3 -fopenmp
LDFLAGS = -L./lib -L/usr/lib -L/usr/local/lib/Matplot++ -ltinyxml2 -lmatplot -lnodesoup -fopenmp

SRCS = cgio.cpp cgproc.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = circlegen

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

test: $(EXEC)
	./$(EXEC) cartman.svg --res 6

leakcheck: $(EXEC)
	valgrind --leak-check=full --track-origins=yes ./$(EXEC) cartman.svg

clean:
	rm -f $(OBJS) $(EXEC) output.jpg

.PHONY: all clean test