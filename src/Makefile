CC=gcc

ROOT:=..
SDL_DIR:=$(ROOT)/SDL2
MARIADB_DIR:=$(ROOT)/mariadb

CFLAGS =-I$(SDL_DIR)/include -I$(MARIADB_DIR)/include -W -Wall -std=gnu99 -O2 -pedantic
LDFLAGS=-L$(SDL_DIR)/lib -lSDL2-2.0 -Wl,-rpath,'$$ORIGIN/SDL2/lib' -L$(MARIADB_DIR)/lib -lmariadb -Wl,-rpath,'$$ORIGIN/mariadb/lib'

TARGET=main
SRC=main.c utils.c database/queue.c database/worker_model.c database/role_model.c database/mysql_base_functions.c database/line_model.c
OBJ=$(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@


main.o : database/worker_model.h

database/worker_model.o : database/worker_model.h utils.h database/queue.h database/mysql_base_functions.h database/role_model.h

database/role_model.o : database/role_model.h

database/line_model.o : database/line_model.h

clean:
	rm -f $(OBJ) $(TARGET)