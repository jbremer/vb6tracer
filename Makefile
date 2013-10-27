CC = gcc
CFLAGS = -Wall -Wextra -O2 -s

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

default: vb6tracer.dll

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

vb6tracer.dll: $(OBJS)
	$(CC) $(CFLAGS) -o $@ -shared $^
