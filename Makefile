CC = gcc
CFLAGS = -Wall -Wextra -O2 -s -std=c99

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

TOOLS = vb6tracer.dll utils/dllinject.exe utils/hello.exe

default: $(TOOLS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

vb6tracer.dll: $(OBJS)
	$(CC) $(CFLAGS) -o $@ -shared $^

utils/%.exe: utils/%.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(TOOLS)
