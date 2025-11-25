ifeq ($(OS),Windows_NT)
    RM = del /F /Q
    EXE = .exe
else
    RM = rm -f
    EXE =
endif

all: libmyreadkey.a

libmyreadkey.a: readlibmyreadkey.o
	@ ar rcs libmyreadkey.a libmyreadkey.o

%.o: %.c
	@ gcc -Wall -Wextra -std=c11 -O2 -I../include -c $< -o $@

test: libmyreadkey.a myReadKey.o
	@ gcc -Wall -Wextra -std=c11 -O2 myReadKey.o -L. -lmyreadkey -o test$(EXE)
	@ ./test$(EXE)

myReadKey.o: myReadKey.c
	@ gcc -Wall -Wextra -std=c11 -O2 -I../include -c myReadKey.c -o myReadKey.o

clean:
	@ $(RM) readlibmyreadkey.o \
	         libmyreadkey.a myReadKey.o test$(EXE)
