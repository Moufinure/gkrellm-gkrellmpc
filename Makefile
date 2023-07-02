#
# $Header: /cvsroot/gkrellmpc/Makefile,v 1.15 2005/03/05 22:38:44 mina Exp $
#

PREFIX ?= /usr/local/
VERSION = $(shell awk '$$1 == "\#define" && $$2 == "VERSION" {gsub("\"", "", $$3); print $$3;}' globals.h)

OBJECTS = gkrellmpc.o mpd.o conf.o playlist.o addlist.o url.o

CC ?= gcc
CFLAGS += -Wall -fPIC `pkg-config gtk+-2.0 --cflags` -DPACKAGE="\"gkrellmpc\""
ifeq ($(enable_nls),1)
	CFLAGS += -DENABLE_NLS
endif
LDFLAGS += `curl-config --libs`

.PHONY: all clean dist install install_lib install_local_lib install_home install_instructions deinstall uninstall test strip

all: gkrellmpc.so

gkrellmpc.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o gkrellmpc.so $(OBJECTS)
ifeq ($(enable_nls),1)
	(cd po && ${MAKE} all)
endif

%.o: %.c %.h globals.h
	$(CC) $(CFLAGS) -c -o $@ $<

strip: gkrellmpc.so
	strip gkrellmpc.so

clean:
	-rm -rf *.o *.so *.tar.gz dist
	(cd po && ${MAKE} clean)

dist: gkrellmpc.c gkrellmpc.h pixmaps Makefile Changelog README.txt
	-rm -rf dist
	mkdir dist
	mkdir dist/gkrellmpc-$(VERSION)
	cp -r README.txt Changelog Makefile *.c *.h pixmaps po dist/gkrellmpc-$(VERSION)/
	-find dist/gkrellmpc-$(VERSION)/ -name CVS -exec rm -rf '{}' ';'
	-find dist/gkrellmpc-$(VERSION)/ -name .cvsignore -exec rm -rf '{}' ';'
	-rm gkrellmpc-$(VERSION).tar.gz
	cd dist && tar czvf ../gkrellmpc-$(VERSION).tar.gz .
	rm -rf dist

install: gkrellmpc.so
	if [ -d $(PREFIX)/lib/gkrellm2/plugins/ -a -w $(PREFIX)/lib/gkrellm2/plugins/ ]; then make install_lib; elif [ -d $(PREFIX)/local/lib/gkrellm2/plugins/ -a -d $(PREFIX)/local/lib/gkrellm2/plugins/ ]; then make install_local_lib; elif [ -d ~/.gkrellm2/plugins/ -a -w ~/.gkrellm2/plugins/ ]; then make install_home; else make install_instructions; fi

install_lib: gkrellmpc.so
	cp gkrellmpc.so $(PREFIX)/lib/gkrellm2/plugins/
ifeq ($(enable_nls),1)
	(cd po && ${MAKE} install)
endif

install_local_lib: gkrellmpc.so
	cp gkrellmpc.so $(PREFIX)/local/lib/gkrellm2/plugins/
ifeq ($(enable_nls),1)
	(cd po && ${MAKE} install)
endif

install_home: gkrellmpc.so
	cp gkrellmpc.so ~/.gkrellm2/plugins/
ifeq ($(enable_nls),1)
	@echo
	@echo Note: NLS .mo files have not been installed
	@echo
endif

install_instructions: gkrellmpc.so
	@echo
	@echo None of the standard locations for gkrellm plugins exist or are writeable by the current user id.
	@echo
	@echo Please manually copy the gkrellmpc.so file to one of your gkrellm plugin directories
	@echo
	@echo Typically system-wide plugins are placed in one of:
	@echo
	@echo "	$(PREFIX)/lib/gkrellm2/plugins/"
	@echo "	$(PREFIX)/local/lib/gkrellm2/plugins/"
	@echo
	@echo And user-specific plugins are placed in:
	@echo
	@echo "	~/.gkrellm2/plugins/"
	@echo
	@echo However, your system may have alternate paths.

deinstall uninstall:
	if [ -f $(PREFIX)/lib/gkrellm2/plugins/gkrellmpc.so ]; then rm -f $(PREFIX)/lib/gkrellm2/plugins/gkrellmpc.so; fi 
	if [ -f $(PREFIX)/local/lib/gkrellm2/plugins/gkrellmpc.so ]; then rm -f $(PREFIX)/local/lib/gkrellm2/plugins/gkrellmpc.so; fi 
	if [ -f ~/.gkrellm2/plugins/gkrellmpc.so ]; then rm -f ~/.gkrellm2/plugins/gkrellmpc.so; fi 

test: gkrellmpc.so
	gkrellm2 -p gkrellmpc.so
