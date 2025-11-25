ifeq ($(OS),Windows_NT)
    RM = del /F /Q
    EXE = .exe
else
    RM = rm -f
    EXE =
endif

all: libmysimplecomputer.a

libmysimplecomputer.a: sc_variables.o sc_memory.o sc_regist.o sc_commandEncoder.o
	@ ar rcs libmysimplecomputer.a sc_variables.o sc_memory.o sc_regist.o sc_commandEncoder.o

%.o: %.c
	@ gcc -Wall -Wextra -std=c11 -O2 -I../include -c $< -o $@

test: libmysimplecomputer.a main.o
	@ gcc -Wall -Wextra -std=c11 -O2 main.o -L. -lmysimplecomputer -o test$(EXE)
	@ ./test$(EXE)

main.o: main.c
	@ gcc -Wall -Wextra -std=c11 -O2 -I../include -c main.c -o main.o

clean:
	@ $(RM) sc_variables.o sc_memory.o sc_regist.o sc_commandEncoder.o \
	         libmysimplecomputer.a main.o test$(EXE)
