#    CS486 - Programming Assignment1, 2022
#    Author: Minos Stavrakakis - csd4120

# the compiler to use
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall 
  
#files to link:
LFLAGS = -lm -lpthread -latomic

# Source files
SRCS = main.c DLList.c HTable.c ULFStack.c

# target file
TARGET = cs486_prog1_exe
  
all: $(TARGET)
  
$(TARGET): $(SRCS)
	@$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LFLAGS)

clean:
	@rm $(TARGET)