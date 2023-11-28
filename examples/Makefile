CC := gcc
INCLUDES := -I../include -I../src -I../deps
LIBS := -L../ -lfsms
TARGET := example

%: %.c
	$(CC) $< $(INCLUDES) $(LIBS) -o example

clean:
	rm example

.PHONY: clean
