CC = gcc
CFLAGS = -O3 -Wall -Wextra -Werror -fanalyzer
TARGET = lab4

all: $(TARGET)

$(TARGET): lab4.c
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET)

clean:
	rm -f $(TARGET)

