CC := gcc
CFLAGS :=
SRCS := zipf.c
LIBS := -lm
TARGET := zifp

.DEFAULT_GOAL := $(TARGET)

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGET)
