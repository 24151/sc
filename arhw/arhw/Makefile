ifeq ($(OS),Windows_NT)
    RM = del /F /Q
    EXE = .exe
	SL = \\
else
    RM = rm -f
    EXE =
	SL = /
endif

CFLAGS = -Wall -Wextra -O3 -Iinclude
target = main
LIBDIR = include
OBJDIR = console

# .a и их .o файлы
LIBS = mySimpleComputer myTerm myBigChars myReadKey out keys

mysimplecomputer_OBJS = mySimpleComputer/sc_commandEncoder.o mySimpleComputer/sc_memory.o mySimpleComputer/sc_regist.o mySimpleComputer/sc_variables.o mySimpleComputer/CPU.o mySimpleComputer/sc_cache.o
myterm_OBJS = myTerm/myTerm.o
mybigchars_OBJS = myBigChars/myBigChars.o
myreadkey_OBJS = myReadKey/myReadKey.o
out_OBJS = console/out.o
keys_OBJS = console/keys.o

.PHONY: all clean run ass

all: $(addprefix $(LIBDIR)/lib,$(addsuffix .a,$(LIBS)))

%.o: %.c
	@ gcc $(CFLAGS) -c $< -o $@

$(LIBDIR)/libmySimpleComputer.a: $(mysimplecomputer_OBJS)
	@ ar rcs $@ $^

$(LIBDIR)/libmyTerm.a: $(myterm_OBJS)
	@ ar rcs $@ $^

$(LIBDIR)/libmyBigChars.a: $(mybigchars_OBJS)
	@ ar rcs $@ $^

$(LIBDIR)/libmyReadKey.a: $(myreadkey_OBJS)
	@ ar rcs $@ $^

$(LIBDIR)/libout.a: $(out_OBJS)
	@ ar rcs $@ $^

$(LIBDIR)/libkeys.a: $(keys_OBJS)
	@ ar rcs $@ $^

$(OBJDIR)/$(target).o: $(OBJDIR)/$(target).c
	@ gcc $(CFLAGS) -c $< -o $@

run: all $(OBJDIR)/$(target).o
	@ gcc $(CFLAGS) $(OBJDIR)/$(target).o -L$(LIBDIR) -Wl,--start-group $(addprefix -l,$(LIBS)) -Wl,--end-group -o $(OBJDIR)/$(target)$(EXE)
	@ ./$(OBJDIR)/$(target)$(EXE)

ass:
	@ cd coursework && make run


clean:
	-@ $(RM) $(OBJDIR)$(SL)$(target).o $(OBJDIR)$(SL)$(target)$(EXE) $(OBJDIR)$(SL)out.o $(OBJDIR)$(SL)keys.o
	-@ $(RM) mySimpleComputer/CPU.o mySimpleComputer/sc_cache.o
	-@ cd coursework/ && make clean && cd ../
	-@ $(RM) $(addprefix $(LIBDIR)$(SL)lib,$(addsuffix .a,$(LIBS)))
	@ $(foreach dir, $(LIBS), \
		$(if $(wildcard $(dir)$(SL)Makefile), \
			$(MAKE) -C $(dir) clean;))
