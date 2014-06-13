CC = i686-w64-mingw32-gcc
LD = i686-w64-mingw32-ld
CFLAGS = -Wall -Wextra -O0 -ggdb -std=c99 -m32
DIRS = -Idistorm3.2-package/include

DISTORM3 = $(wildcard distorm3.2-package/src/*.c)
DISTORM3OBJ = $(DISTORM3:.c=.o)

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

TOOLS = vb6tracer.dll utils/dllinject.exe utils/hello.exe

default: $(TOOLS)

%.o: %.c
	$(CC) $(DIRS) $(CFLAGS) -c $^ -o $@

vb6tracer.dll: $(OBJS) $(DISTORM3OBJ)
	$(CC) $(CFLAGS) -o $@ -shared $^

utils/%.exe: utils/%.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(TOOLS) $(DISTORM3OBJ)
