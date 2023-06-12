CC=gcc
SRC=$(wildcard ./*.c)
TARGET=example

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -I../include -I../src -I../deps $(SRC) -o $(TARGET) -L../ -lfsms

clean:
	rm $(TARGET)
