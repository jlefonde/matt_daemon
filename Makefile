NAME = MattDaemon

SOURCES_DIR = srcs
HEADERS_DIR = includes
OBJECTS_DIR = objs

SOURCES = main.cpp \
			utils.cpp \
			Config.cpp \
			DaemonConfig.cpp \
			ServerConfig.cpp \
			LoggerConfig.cpp \
			Daemon.cpp \
			TintinReporter.cpp \
			Server.cpp

OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(SOURCES:.cpp=.o))

CFLAGS = -g -Wall -Wextra -Werror -I$(HEADERS_DIR)

all: $(NAME)

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	@mkdir -p $(@D)
	c++ $(CFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	c++ $(CFLAGS) $(OBJECTS) -o $@

clean:
	pgrep $(NAME) | sudo xargs -r kill -9
	sudo rm -f /var/lock/matt_daemon.lock
	sudo rm -f /var/run/matt_daemon.pid
	$(RM) -r $(OBJECTS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re