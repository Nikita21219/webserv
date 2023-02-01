NAME = webserv
CC = c++

#CPPFLAGS = -g3 -Wall -Wextra -Werror -Wshadow -Wno-shadow -fsanitize=address -std=c++98
CPPFLAGS = -Wall -Wextra -Werror -std=c++98

OBJDIR = ./objs
VPATH = sources/
HPATH = headers/

HEADER = $(wildcard $(addsuffix /*.hpp,$(HPATH)))
OBJ = $(addprefix $(OBJDIR)/,$(notdir $(SRC:.cpp=.o)))
SRC = $(wildcard $(addsuffix /*.cpp,$(VPATH)))

all:	$(NAME)

$(OBJDIR)/%.o :	%.cpp $^ $(HEADER)
	@$(CC) $(CPPFLAGS) -iquote headers -c $< -o $@

$(NAME):	Makefile $(OBJDIR) $(OBJ)
	@$(CC) $(CPPFLAGS) -o $(NAME) $(OBJ)
	@echo "$(NAME) compile"

$(OBJDIR):
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi

clean:
	@if [ -d $(OBJDIR) ] ; then rm -r $(OBJDIR); fi
	@echo "clean $(NAME) done"


fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re