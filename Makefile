NAME        = webserv
CPP         = c++
CPPFLAGS    = -Wall -Wextra -Werror -std=c++98
RM          = rm -rf

# ---------------- Dirs ----------------
SRC_DIR     = src
INC_DIR     = include
OBJ_DIR     = build

# ---------------- Files ---------------
# Explicitly list your files with their subfolder paths
FILES       = main.cpp \
              Log/Log.cpp

SRCS        = $(addprefix $(SRC_DIR)/, $(FILES))
OBJS        = $(addprefix $(OBJ_DIR)/, $(FILES:.cpp=.o))

# Adding -I for root include dir only, forcing everyone to use the folder prefix, keeping the codebase consistent.
INCLUDES    = -I$(INC_DIR)

# ---------------- Rules ---------------

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)
	@echo "Webserver built! 🚀"

# Using find to track ALL headers in ALL subdirectories
HEADERS     = $(shell find $(INC_DIR) -name "*.hpp" -o -name "*.h")

# The object rule now depends on that list of headers
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re