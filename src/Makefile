CC=gcc

ROOT:=..
SDL_DIR:=$(ROOT)/SDL2
MARIADB_DIR:=$(ROOT)/mariadb

CFLAGS =-I$(SDL_DIR)/include -I$(MARIADB_DIR)/include -W -Wall -std=gnu99 -O2 -pedantic
LDFLAGS=-L$(SDL_DIR)/lib -lSDL2-2.0 -Wl,-rpath,'$$ORIGIN/SDL2/lib' -L$(MARIADB_DIR)/lib -lmariadb -Wl,-rpath,'$$ORIGIN/mariadb/lib'

TARGET=main
SRC=main.c
OBJ=$(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -f $(OBJ) $(TARGET)