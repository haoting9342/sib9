#CC = gcc
#CFLAGS =  -g -shared -fPIC  #-Wl,-Bsymbolic-functions
LIBRARY = -lyaml
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o) 
NAME = yaml_wrapper.so
INCL_DIR= -I. -I../..

TARGS= all clean

all: $(NAME)
	cp $(NAME) $(OUTPUT_DIR)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LIBRARY) $(SHARED_CFLAGS) $(INCL_DIR)


$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) $(INCL_DIR) $(SHARED_CFLAGS)


clean:
	rm -f $(OBJ) $(NAME)
	rm -f $(OUTPUT_DIR)/$(NAME)
	
.PHONY: $(TARGS)