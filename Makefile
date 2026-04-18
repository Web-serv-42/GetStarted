NAME        = webserv
CPP         = c++
CPPFLAGS    = -Wall -Wextra -Werror -std=c++98
RM          = rm -rf

# ---------------- Dirs ----------------
SRC_DIR     = src
INC_DIR     = include
OBJ_DIR     = build

# ---------------- Files ---------------
# 1. Find all .cpp files
SRCS        = $(shell find $(SRC_DIR) -name "*.cpp")

# 2. Transform src/Core/Timer.cpp -> build/Core/Timer.o
# This is the magic fix! 
OBJS        = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Tracking all headers for recompilation
HEADERS     = $(shell find $(INC_DIR) -name "*.hpp" -o -name "*.h")

INCLUDES    = -I$(INC_DIR)

# ---------------- Rules ---------------

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)
	@echo "Webserver built! 🚀"

# This rule now matches the paths in $(OBJS)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re