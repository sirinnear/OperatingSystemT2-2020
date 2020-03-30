CC = gcc
CFLAGS = -Wall -W -pedantic -g
TARGET = virtmem
all: $(TARGET)
$(TARGET):
	$(CC) $(CFLAGS) -o page_fault_counter $(TARGET).c
clean:
	/bin/rm -f a4.zip
	/bin/rm -f page_fault_counter

zip: clean
	/usr/bin/zip a4.zip ./*
