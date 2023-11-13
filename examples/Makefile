CC ?= gcc
INCLUDES := -I../include -I../src -I../deps
LIBS := -L../ -lfsms
TARGET := example

scheduler: $(SRC)
	$(CC) scheduler.c $(INCLUDES) $(LIBS) $(SRC) -o example

switch: $(SRC)
	$(CC) switch.c $(INCLUDES) $(LIBS) $(SRC) -o example

switch-complex: $(SRC)
	$(CC) switch-complex.c $(INCLUDES) $(LIBS) $(SRC) -o example

clean:
	rm example

.PHONY: scheduler switch clean
