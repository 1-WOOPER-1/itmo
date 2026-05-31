CC = gcc
CFLAGS = -O3 -Wall -Wextra -Werror -fanalyzer
TARGET = lab3

all: $(TARGET)

$(TARGET): lab3.c
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET)

clean:
	rm -f $(TARGET)

