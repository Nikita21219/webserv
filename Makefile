NAME = webserv
CC = c++

CPPFLAGS = -g -Wall -Wextra -Werror -Wshadow -Wno-shadow -Wunused-command-line-argument -fsanitize=address -std=c++98

OBJDIR = ./objs
VPATH = .

HEADER = $(wildcard $(addsuffix /*.hpp,$(VPATH)))
OBJ = $(addprefix $(OBJDIR)/,$(notdir $(SRC:.cpp=.o)))
SRC = $(wildcard $(addsuffix /*.cpp,$(VPATH)))

all:	$(NAME)

$(OBJDIR)/%.o :	%.cpp $^ $(HEADER)
	@$(CC) $(CPPFLAGS) -c $< -o $@

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