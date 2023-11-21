#CC = gcc
#CFLAGS =  -g -shared -fPIC 
LIBRARY = -lpthread -lc -L./lib -L./ -lpcielib
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o) 
NAME = tsn_mqtt_time_domain.so
INCL_DIR= -I. -I../.. -I..

TARGS= all clean

all: $(NAME)
	cp $(NAME) $(OUTPUT_DIR)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIBRARY) -o $(NAME) $(SHARED_CFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) $(INCL_DIR) $(SHARED_CFLAGS)


clean:
	rm -f $(OBJ) $(NAME)
	rm -f $(OUTPUT_DIR)/$(NAME)
	
.PHONY: $(TARGS)