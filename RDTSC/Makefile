CC := gcc
CFLAGS :=
SRCS := rdtsc.c
LIBS := 
TARGET := rdtsc

.DEFAULT_GOAL := $(TARGET)

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGET)
