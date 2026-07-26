NAME        = webserv
CPP         = c++
CPPFLAGS    = -Wall -Wextra -Werror -std=c++98 -g
RM          = rm -rf

# ---------------- Dirs ----------------
SRC_DIR     = src
INC_DIR     = include
OBJ_DIR     = build
TMP_DIR     = tmp
UPLOAD_DIR  = uploads

# ---------------- Files ---------------
# 1. Find all .cpp files
SRCS        = $(shell find $(SRC_DIR) -name "*.cpp")

# 2. Transform src/Core/Timer.cpp -> build/Core/Timer.o
OBJS        = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Tracking all headers for recompilation
HEADERS     = $(shell find $(INC_DIR) -name "*.hpp" -o -name "*.h")

INCLUDES    = -I$(INC_DIR)

# ---------------- Rules ---------------

all: $(NAME)

$(NAME): $(OBJS)
	@mkdir -p $(TMP_DIR) $(UPLOAD_DIR)
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)
	@echo "Webserver built! 🚀"

# This rule matches the paths in $(OBJS)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJ_DIR)
	$(RM) $(TMP_DIR)/*

fclean: clean
	$(RM) $(NAME) $(TMP_DIR) $(UPLOAD_DIR)

re: fclean all

.PHONY: all clean fclean re