# User config
CC = gcc-12
CFLAGS = -Wall -Wextra

INC_DIR += app
INC_DIR += app/inc

# Source directories
SRC_DIR += test
SRC_DIR += app/src

# Executable name 
EXECUTABLE = mcut_test.out

# Auto resolve files from given dir paths
INCLUDES = $(foreach dir, $(INC_DIR), -I $(dir))
SOURCES = $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c, %.o, $(SOURCES))

# Flavors
.PHONY: depend clean test

test: $(EXECUTABLE)
	@echo $(EXECUTABLE) ready

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(EXECUTABLE) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) $(OBJS)
	$(RM) $(EXECUTABLE)
