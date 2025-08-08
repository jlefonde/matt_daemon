NAME = matt_daemon

SOURCES_DIR = srcs
HEADERS_DIR = includes
OBJECTS_DIR = objs

SOURCES = main.cpp \
			TintinReporter.cpp \
			Daemon.cpp

OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(SOURCES:.cpp=.o))

CFLAGS = -g -Wall -Wextra -Werror -I$(HEADERS_DIR)

all: $(NAME)

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	@mkdir -p $(@D)
	c++ $(CFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	c++ $(CFLAGS) $(OBJECTS) -o $@

clean:
	pgrep $(NAME) | xargs -r kill -9
# 	rm /var/lock/$(NAME).lock
	$(RM) -r $(OBJECTS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re