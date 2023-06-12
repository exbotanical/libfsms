CC=gcc
OBJFILES=$(wildcard ./*.c)
TARGET=example

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) -I../src -I../deps $(OBJFILES) -o $(TARGET) -L../ -lfsms

clean:
	rm $(TARGET)
