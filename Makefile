#
# Tiny C Compiler Makefile
#

TOP ?= .
include $(TOP)/config.mak

CONFIG_816=yes
CONFIG_CROSS=yes

ifdef CONFIG_WIN32
CFLAGS+=-m32
endif

CFLAGS+=-g -Wall
CFLAGS+=-Wno-unused-but-set-variable  \
		-Wno-format-overflow \
		-Wno-array-bounds \
		-Wno-format-truncation \
		-Wno-pointer-sign \
    -Wno-maybe-uninitialized \
    -Wno-unused-result
CFLAGS_P=$(CFLAGS) -pg -static -DCONFIG_TCC_STATIC
LIBS_P=

ifneq ($(GCC_MAJOR),2)
CFLAGS+=-fno-strict-aliasing
endif

ifeq ($(ARCH),i386)
CFLAGS+=-mpreferred-stack-boundary=2
ifeq ($(GCC_MAJOR),2)
CFLAGS+=-malign-functions=0
else
CFLAGS+=-falign-functions=0
ifneq ($(GCC_MAJOR),3)
CFLAGS+=-Wno-pointer-sign -Wno-sign-compare -D_FORTIFY_SOURCE=0
endif
endif
endif

ifndef CONFIG_WIN32
LIBS=-lm
ifneq ($(OS),Windows_NT)
LIBS+=-ldl
endif
endif

LIBTCC1=libtcc1.a
BCHECK_O=bcheck.o
CROSS_TARGET=-DTCC_TARGET_816


ifeq ($(TOP),.)

CORE_FILES = tcc.c libtcc.c tccpp.c tccgen.c tccelf.c tccasm.c \
    tcc.h config.h libtcc.h tcctok.h
816_FILES = $(CORE_FILES) 816-gen.c

CROSS_FILES=$(816_FILES)
PROGS=816-tcc$(EXESUF)

ifndef CONFIG_CROSS
$(error "Non-cross build is not supported for WDC 65816")
endif

all: $(PROGS) $(LIBTCC1)

# Cross Tiny C Compilers
816-tcc$(EXESUF): $(816_FILES)
	$(CC) -o $@ $< $(CROSS_TARGET) $(CFLAGS) $(LIBS)

# libtcc generation
libtcc.o: $(CROSS_FILES)
	$(CC) -o $@ -c libtcc.c $(CROSS_TARGET) $(CFLAGS)

libtcc.a: libtcc.o
	$(AR) rcs $@ $^

# profiling version
816-tcc_p$(EXESUF): $(CROSS_FILES)
	$(CC) -o $@ $< $(CROSS_TARGET) $(CFLAGS_P) $(LIBS_P)

# TinyCC runtime libraries
LIBTCC1_OBJS=lib/libtcc1.o
LIBTCC1_CC=$(CC)

%.o: %.c
	$(LIBTCC1_CC) -o $@ -c $< -O2 -Wall $(CROSS_TARGET)

%.o: %.S
	$(LIBTCC1_CC) -o $@ -c $<

libtcc1.a: $(LIBTCC1_OBJS)
	$(AR) rcs $@ $^

bcheck.o: bcheck.c
	$(CC) -o $@ -c $< -O2 -Wall

# install
TCC_INCLUDES = stdarg.h stddef.h stdbool.h float.h varargs.h tcclib.h
INSTALL=install

install: $(PROGS) $(LIBTCC1) $(BCHECK_O) libtcc.a tcc.1 tcc-doc.html
	mkdir -p "$(bindir)"
	$(INSTALL) -s -m755 $(PROGS) "$(bindir)"
	mkdir -p "$(mandir)/man1"
	$(INSTALL) tcc.1 "$(mandir)/man1"
	mkdir -p "$(tccdir)"
	mkdir -p "$(tccdir)/include"
ifneq ($(LIBTCC1),)
	$(INSTALL) -m644 $(LIBTCC1) "$(tccdir)"
endif
ifneq ($(BCHECK_O),)
	$(INSTALL) -m644 $(BCHECK_O) "$(tccdir)"
endif
	$(INSTALL) -m644 $(addprefix include/,$(TCC_INCLUDES)) "$(tccdir)/include"
	mkdir -p "$(docdir)"
	$(INSTALL) -m644 tcc-doc.html "$(docdir)"
	mkdir -p "$(libdir)"
	$(INSTALL) -m644 libtcc.a "$(libdir)"
	mkdir -p "$(includedir)"
	$(INSTALL) -m644 libtcc.h "$(includedir)"

uninstall:
	rm -fv $(foreach P,$(PROGS),"$(bindir)/$P")
	rm -fv $(foreach P,$(LIBTCC1) $(BCHECK_O),"$(tccdir)/$P")
	rm -fv $(foreach P,$(TCC_INCLUDES),"$(tccdir)/include/$P")
	rm -fv "$(docdir)/tcc-doc.html" "$(mandir)/man1/tcc.1"
	rm -fv "$(libdir)/libtcc.a" "$(includedir)/libtcc.h"

docs: docs/html/index.html

docs/html/index.html:
	@rm -rf docs/html
	@git submodule update --init
	@doxygen docs/Doxyfile

# tar release (use 'make -k tar' on a checkouted tree)
TCC-VERSION=tcc-$(shell cat VERSION)
tar:
	rm -rf /tmp/$(TCC-VERSION)
	cp -r . /tmp/$(TCC-VERSION)
	( cd /tmp ; tar zcvf ~/$(TCC-VERSION).tar.gz $(TCC-VERSION) --exclude CVS )
	rm -rf /tmp/$(TCC-VERSION)

config.mak:
	@echo Running configure ...
	@./configure

# clean
clean:
	rm -f $(PROGS) *.o lib/*.o *.a *.out

distclean: clean
	rm -rf config.h config.mak docs/html

endif # ifeq ($(TOP),.)
