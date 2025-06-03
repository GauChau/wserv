exec = WEBSERV!
src = srcs/webserv.cpp  srcs/conf_parser.cpp srcs/main.cpp srcs/request.cpp
src_obj = $(src.cpp=.o)
cc = c++
flags = -Wall -Wextra -std=c++98

all: $(exec)

$(exec): $(src_obj)
	$(cc) $(flags) $(src) -I./srcs/ -o $(exec)

fclean: clean
	rm -f $(exec)

re: fclean all

.PHONY: all clean fclean
