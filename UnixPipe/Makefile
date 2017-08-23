CC := gcc
CFLAGS :=
SRCS := pipe_shell.c pipe_shell.h
LIBS := -lreadline
TARGET := shell

.DEFAULT_GOAL := $(TARGET)

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGET)
