#CC = gcc
NAME = irgb.reader.so
INCL_DIR = -I./include/  -I. -I.. -I../..
LIB_DIR = -L./lib/ -L./
LIBRARY = -lpcielib -lm -lmosquitto
#exCFLAGS = -shared -std=gnu99
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o) 


all: $(NAME)
	cp $(NAME) $(OUTPUT_DIR)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LIBRARY) $(LIB_DIR) $(SHARED_CFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) $(INCL_DIR) $(SHARED_CFLAGS)

clean:
	rm -f $(OBJ) $(NAME)
	rm -f $(OUTPUT_DIR)/$(NAME)
