MAKEFLAGS += -rR

ifeq (Windows_NT, $(OS))
	KERNEL := Windows_NT
	ARCH   := x86_64
else
	KERNEL    := $(shell uname -s)
	ARCH      := $(shell uname -m)
endif

ifneq ($(findstring ppc64, $(ARCH)), )
	ARCH = -mtune=native
else
	ARCH = -march=native -mtune=native
endif

ifeq (Windows_NT, $(KERNEL))
	LTO =
else
	LTO = -flto=auto
endif

MKDIR_P    = mkdir -p
RM         = rm
INSTALL    = install

CC         = cc
CSTANDARD  = -std=c11
CWARNINGS  = -Wall -Wextra -Wshadow -pedantic -Wno-unused-result -Wvla
COPTIMIZE  = -O2 $(ARCH) $(LTO)

ifeq ($(DEBUG), yes)
	CDEBUG = -g3 -ggdb
else ifeq ($(DEBUG), thread)
	CDEBUG = -g3 -ggdb -fsanitize=thread,undefined
else ifeq ($(DEBUG), address)
	CDEBUG = -g3 -ggdb -fsanitize=address,undefined
else ifeq ($(DEBUG), )
	CDEBUG = -DNDEBUG
endif

CFLAGS     = $(CSTANDARD) $(CWARNINGS) $(COPTIMIZE) $(CDEBUG) -Iinclude
LDFLAGS    = $(CFLAGS)
LDLIBS     = -lm

ifneq ($(DEBUG), )
	LDFLAGS += -rdynamic
endif

ifneq ($(STATIC), )
	LDFLAGS += -static
endif

ifeq ($(SIMD), avx2)
	CFLAGS += -DAVX2 -mavx2
endif

TT        ?= 256

SRC = reversefish.c bitboard.c magicbitboard.c util.c move.c position.c\
      search.c

OBJ = $(patsubst %.c,obj/%.o,$(SRC))

BIN = reversefish

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib64
MANPREFIX = $(PREFIX)/share
MANDIR = $(MANPREFIX)/man
MAN6DIR = $(MANDIR)/man6

all: reversefish

reversefish: $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

obj/%.o: src/%.c dep/%.d
	@$(MKDIR_P) obj
	$(CC) $(CFLAGS) -c $< -o $@

dep/%.d: src/%.c Makefile
	@$(MKDIR_P) dep
	@$(CC) -MM -MT "$@ $(<:src/%.c=obj/%.o)" $(CFLAGS) $< -o $@

obj/search.o: CFLAGS += -DTT=$(TT)

install: all
	$(MKDIR_P) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 reversefish $(DESTDIR)$(BINDIR)

uninstall:
	$(RM) -f $(DESTDIR)$(BINDIR)/reversefish

clean:
	$(RM) -rf obj dep

-include $(DEP)
.PRECIOUS: dep/%.d
.SUFFIXES: .c .h .d
.PHONY: all clean install uninstall

