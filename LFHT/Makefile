CC := gcc
CFLAGS := -std=gnu99 -g
SRCS := hashtable.c hashtable.h jenkins_hash.c test_main.c util.h
LIBS := -lpthread
TARGET := test_main

.DEFAULT_GOAL := $(TARGET)

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGET)
