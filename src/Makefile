CC=gcc

ROOT:=..
SDL_DIR:=$(ROOT)/SDL2
MARIADB_DIR:=$(ROOT)/mariadb

CFLAGS =-I$(SDL_DIR)/include -I$(MARIADB_DIR)/include -W -Wall -std=gnu99 -O2 -pedantic
LDFLAGS=-L$(SDL_DIR)/lib -lSDL2-2.0 -Wl,-rpath,'$$ORIGIN/SDL2/lib' -L$(MARIADB_DIR)/lib -lmariadb -Wl,-rpath,'$$ORIGIN/mariadb/lib'

TARGET=main
SRC=$(wildcard *.c) $(wildcard database/*.c)
OBJ=$(SRC:.c=.o)

DTARGET=debug.db
DSRC=$(wildcard debug/*.c)
DHEADERS=$(wildcard debug/*.h)
DOBJ=$(DSRC:.c=.o)

all: $(TARGET)

check_db: $(DTARGET)
	valgrind --show-leak-kinds=all $(DTARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
$(DTARGET) : $(DOBJ) $(filter-out main.o, $(OBJ)) 
	$(CC) -o $@ -g $^ $(LDFLAGS)


%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@


debug/debug.o : $(DHEADERS)

database/worker_model.o : database/worker_model.h utils.h database/queue.h database/mysql_base_functions.h database/role_model.h

database/role_model.o : database/role_model.h

database/line_model.o : database/line_model.h

database/worker_work_day_model.o : database/worker_work_day_model.h database/worker_model.h database/line_model.h database/work_time_list.h

database/work_time_list.o :  database/work_time_list.h

clean:
	rm -f $(OBJ) $(TARGET) $(DOBJ) $(DTARGET)