CC := gcc
CFLAGS :=
SRCS := hashtable.c hashtable.h jenkins_hash.c test_main.c
LIBS := -lpthread
TARGET := test_main

.DEFAULT_GOAL := $(TARGET)

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGET)
