#CC = gcc
#CFLAGS =  -g -shared -fPIC 
LIBRARY = -lmosquitto -luuid
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o) 
NAME = mosquittoConnector.so

TARGS= all clean

all: $(NAME)
	cp $(NAME) $(OUTPUT_DIR)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LIBRARY) $(SHARED_CFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) $(INCL_DIR) $(SHARED_CFLAGS)

clean:
	rm -f $(OBJ) $(NAME)
	rm -f $(OUTPUT_DIR)/$(NAME)
	
.PHONY: $(TARGS)