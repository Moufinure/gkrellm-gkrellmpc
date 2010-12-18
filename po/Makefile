#
# $Header: /cvsroot/gkrellmpc/po/Makefile,v 1.1 2005/03/05 22:27:27 mina Exp $
#
MSGFMT = msgfmt
FILES_PO:=$(wildcard *.po)
FILES_MO:=$(FILES_PO:.po=.mo)

PREFIX ?= "/usr/share"
LOCALEDIR ?= $(PREFIX)/locale
PKGNAME ?= gkrellmpc

.PHONY: install

ifeq ($(enable_nls),1)
all: mo-files

mo-files: $(FILES_MO)

install: 
	$(MAKE) all
	for f in *.mo ; do mkdir -p \
		$(LOCALEDIR)/`basename $$f .mo`/LC_MESSAGES ; \
		cp $$f $(LOCALEDIR)/`basename $$f .mo`/LC_MESSAGES/$(PKGNAME).mo ; done

uninstall: 
	for f in $(FILES_MO) ; do \
		rm $(LOCALEDIR)/`basename $$f .mo`/LC_MESSAGES/$(PKGNAME).mo ; done

%.mo: %.po
	$(MSGFMT) -f -v -o $@ $<

else

all:

install:

endif

pot:
	xgettext -d $(PKGNAME) --keyword=_ --keyword=N_ -C -o $(PKGNAME).pot ../*.c

clean:
	$(RM) $(FILES_MO)

