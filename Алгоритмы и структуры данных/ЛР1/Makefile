CC = gcc
CFLAGS = -O3 -Wall -Wextra -Werror -fanalyzer
TARGET = lab1

all: $(TARGET)

$(TARGET): lab1.c
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET)

clean:
	rm -f $(TARGET)

