CC=gcc
RC=windres
LDFLAGS=-mwindows
CFLAGS=-g -I ./include
TARGET=win32_test.exe
SRC=$(wildcard *.c)
default:
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS) $(LDFLAGS)
