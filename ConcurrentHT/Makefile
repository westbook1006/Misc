CC := gcc
CFLAGS := -g -Wall
LIBS := -lpthread

SRCS := test_main.c \
		concurrent_ht.c

MAIN := test_main
OBJS = $(SRCS:.c=.o)

.PHONY: clean all

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIBS)

clean:
	rm -f $(TARGET)
