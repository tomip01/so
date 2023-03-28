CC=gcc

ARCH := $(shell uname -m)

BINS=hai

OBJS=$(BINS:=.o)

all: $(BINS)

# Dependencias
hai.o: hai.c

clean:
	rm -rf $(OBJS) $(BINS)
