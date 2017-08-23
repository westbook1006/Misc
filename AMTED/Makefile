CC := gcc
CFLAGS :=
SRCS := amted_server.c threadpool.c
LIBS := -lpthread
TARGET := amted_server

.DEFAULT_GOAL := $(TARGET)

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGET)
