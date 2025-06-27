EXEC    = WEBSERV
SRC     = srcs/webserv.cpp srcs/conf_parser.cpp srcs/main.cpp srcs/request.cpp
OBJ     = $(SRC:.cpp=.o)
CC      = c++
FLAGS   = -Wall -Wextra -std=c++98

all: $(EXEC)
	echo "Running ./conf/script.sh..."
	./conf/script.sh

# link the object files to create the executable
$(EXEC): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -I./srcs/ -o $(EXEC)

# compile .cpp to .o
%.o: %.cpp
	$(CC) $(FLAGS) -I./srcs/ -c $< -o $@

clean:
	rm -f $(OBJ)

# Full clean including executable
fclean: clean
	rm -f $(EXEC)
	rm -rf ./www
	rm -rf ./uploads
	rm -rf ./cgi-bin
	rm -rf ./srcs/build
	rm -rf ./public

re: fclean all
