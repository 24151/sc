ifeq ($(OS),Windows_NT)
    RM = del /F /Q
    EXE = .exe
else
    RM = rm -f
    EXE =
endif

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -I../include

LIB_DIR = myBigChars
LIB_NAME = $(LIB_DIR)/libmybigchars.a

all: $(LIB_NAME)

$(LIB_NAME): myBigChars.o
	@ ar rcs $(LIB_NAME) myBigChars.o

myBigChars.o: myBigChars.c ../include/myBigChars.h ../include/myTerm.h
	@ $(CC) $(CFLAGS) -c myBigChars.c -o myBigChars.o

clean:
	@ $(RM) myBigChars.o

.PHONY: all clean
