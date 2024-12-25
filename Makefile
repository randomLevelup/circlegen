CC = g++
CFLAGS = -Wall -Wextra -Wpedantic -I./include -I/usr/local/include/
LDFLAGS = -L./lib -L/usr/lib -L/usr/local/lib/Matplot++ -ltinyxml2 -lmatplot -lnodesoup

SRCS = cgio.cpp cgproc.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = circlegen

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

test: $(EXEC)
	./$(EXEC) cartman.svg

clean:
	rm -f $(OBJS) $(EXEC) output.jpg

.PHONY: all clean test