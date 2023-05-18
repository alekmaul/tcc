#
# Tiny C Compiler Makefile
#

TOP ?= .
include $(TOP)/config.mak

TARGET=
CFLAGS=-Wall -g \
		-Wno-unused-but-set-variable  \
		-Wno-format-overflow

ifeq ($(ARCH),x86-64)
TARGET=-DTCC_TARGET_X86_64
CFLAGS+=-Wno-pointer-sign
endif

ifndef CONFIG_WIN32
LIBS=-lm
ifndef CONFIG_NOLDL
LIBS+=-ldl
endif
ifneq ($(ARCH),x86-64)
BCHECK_O=bcheck.o
endif
endif
CFLAGS_P=$(CFLAGS) -pg -static -DCONFIG_TCC_STATIC
LIBS_P=

ifneq ($(GCC_MAJOR),2)
CFLAGS+=-fno-strict-aliasing
endif

ifeq ($(ARCH),i386)
CFLAGS+=-mpreferred-stack-boundary=2
ifeq ($(GCC_MAJOR),2)
CFLAGS+=-m386 -malign-functions=0
else
CFLAGS+=-march=i386 -falign-functions=0
ifneq ($(GCC_MAJOR),3)
CFLAGS+=-Wno-pointer-sign -Wno-sign-compare -D_FORTIFY_SOURCE=0
endif
endif
endif

ifeq ($(TOP),.)

PROGS=816-tcc$(EXESUF)

all: $(PROGS) $(LIBTCC1) $(BCHECK_O) tcc-doc.html tcc.1

TCC_CORE_FILES = tcc.c tccelf.c tccasm.c tcctok.h libtcc.h config.h


# Cross Tiny C Compilers
816-tcc$(EXESUF): $(TCC_CORE_FILES) 816-gen.c
	$(CC) -o $@ $< -DTCC_TARGET_816 $(CFLAGS) $(LIBS)

# libtcc generation and test
libtcc.o: tcc.c i386-gen.c
ifdef CONFIG_WIN32
	$(CC) -o $@ -c $< -DTCC_TARGET_PE -DLIBTCC $(CFLAGS)
else
	$(CC) -o $@ -c $< $(TARGET) -DLIBTCC $(CFLAGS)
endif

libtcc.a: libtcc.o
	$(AR) rcs $@ $^

libtcc_test$(EXESUF): tests/libtcc_test.c libtcc.a
	$(CC) -o $@ $^ -I. $(CFLAGS) $(LIBS)

libtest: libtcc_test$(EXESUF) $(LIBTCC1)
	./libtcc_test$(EXESUF) lib_path=.

# profiling version
tcc_p: tcc.c
	$(CC) -o $@ $< $(CFLAGS_P) $(LIBS_P)

# windows utilities
tiny_impdef$(EXESUF): win32/tools/tiny_impdef.c
	$(CC) -o $@ $< $(CFLAGS)
tiny_libmaker$(EXESUF): win32/tools/tiny_libmaker.c
	$(CC) -o $@ $< $(CFLAGS)

# TinyCC runtime libraries
LIBTCC1_OBJS=libtcc1.o
LIBTCC1_CC=$(CC)
VPATH+=lib
ifdef CONFIG_WIN32
# for windows, we must use TCC because we generate ELF objects
LIBTCC1_OBJS+=crt1.o wincrt1.o dllcrt1.o dllmain.o chkstk.o
LIBTCC1_CC=./tcc.exe -Bwin32 -DTCC_TARGET_PE
VPATH+=win32/lib
endif
ifeq ($(ARCH),i386)
LIBTCC1_OBJS+=alloca86.o alloca86-bt.o
endif

%.o: %.c
	$(LIBTCC1_CC) -o $@ -c $<  -O2 -Wall

%.o: %.S
	$(LIBTCC1_CC) -o $@ -c $<

libtcc1.a: $(LIBTCC1_OBJS)
	$(AR) rcs $@ $^

bcheck.o: bcheck.c
	$(CC) -o $@ -c $< -O2 -Wall

# install
TCC_INCLUDES = stdarg.h stddef.h stdbool.h float.h varargs.h tcclib.h
INSTALL=install

ifndef CONFIG_WIN32
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

else
install: $(PROGS) $(LIBTCC1) libtcc.a tcc-doc.html
	mkdir -p "$(tccdir)"
	mkdir -p "$(tccdir)/lib"
	mkdir -p "$(tccdir)/include"
	mkdir -p "$(tccdir)/examples"
	mkdir -p "$(tccdir)/doc"
	mkdir -p "$(tccdir)/libtcc"
	$(INSTALL) -s -m755 $(PROGS) "$(tccdir)"
	$(INSTALL) -m644 $(LIBTCC1) win32/lib/*.def "$(tccdir)/lib"
	cp -r win32/include/. "$(tccdir)/include"
	cp -r win32/examples/. "$(tccdir)/examples"
	$(INSTALL) -m644 $(addprefix include/,$(TCC_INCLUDES)) "$(tccdir)/include"
	$(INSTALL) -m644 tcc-doc.html win32/tcc-win32.txt"$(tccdir)/doc"
	$(INSTALL) -m644 libtcc.a libtcc.h "$(tccdir)/libtcc"
endif

# documentation and man page
t2hinstalled := $(shell command -v texi2html 2> /dev/null)
tcc-doc.html: tcc-doc.texi
ifndef t2hinstalled
	@echo "texi2html is not installed, documentation will be not generated.";
else
	-texi2html -monolithic -number-sections $< >/dev/null 2>&1
endif

tcc.1: tcc-doc.texi
	-./texi2pod.pl $< tcc.pod
	-pod2man --section=1 --center=" " --release=" " tcc.pod > $@

# tar release (use 'make -k tar' on a checkouted tree)
TCC-VERSION=tcc-$(shell cat VERSION)
tar:
	rm -rf /tmp/$(TCC-VERSION)
	cp -r . /tmp/$(TCC-VERSION)
	( cd /tmp ; tar zcvf ~/$(TCC-VERSION).tar.gz $(TCC-VERSION) --exclude CVS )
	rm -rf /tmp/$(TCC-VERSION)

# in tests subdir
test clean :
	$(MAKE) -C tests $@

# clean
clean: local_clean
local_clean:
	rm -vf $(PROGS) tcc_p tcc.pod *~ *.o *.a *.out libtcc_test$(EXESUF) tcc-doc.html

distclean: clean
	rm -vf config.h config.mak config.texi tcc.1 tcc-doc.html

endif
# ifeq ($(TOP),.)