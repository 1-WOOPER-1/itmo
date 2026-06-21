CC = gcc
CFLAGS = -O3 -Wall -Wextra -Werror -fanalyzer
LIBS = -lm
TARGET = lab5

all: $(TARGET)

$(TARGET): lab5.c
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)