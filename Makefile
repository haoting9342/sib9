# make current folder with gcc 
# make clean to remove all .o and executable files
# make run to run the executable file
# make debug to run the executable file with gdb
# make valgrind to run the executable file with valgrind
# make all to make the executable file
# make library shall include -lyaml -ldl

export CC = gcc
export CFLAGS = -Wall -Wextra -Werror -g -rdynamic -Wl,-rpath,.:./lib:./plugins
export SHARED_CFLAGS = -shared -std=gnu99 -fPIC

LIBRARY = -lyaml -ldl  -luuid 
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
NAME = dstt

export OUTPUT_DIR  = $(CURDIR)/lib


all:  yaml_wrapper plugins $(NAME)

yaml_wrapper:
	make -C plugins/yaml_wrapper $(MAKECMDGOALS)

plugins: 
	make -C plugins all

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LIBRARY) -L./lib -l:yaml_wrapper.so

clean:
	rm -f $(OBJ) $(NAME)
	make -C plugins clean

run: $(NAME)
	./$(NAME)

debug: $(NAME)
	gdb ./$(NAME)

.PHONY: all clean run debug plugins


