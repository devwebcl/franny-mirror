.PHONY: clean

### configuration
PREFIX=/usr/local/franny

### global variables
PACKAGE=franny
VERSION=1.1.3

### distributed files
SCRIPTS=insert.sh extract.sh
SOURCE_FILES=*.cpp *.h
EXTRA_FILES=Makefile copying

### needed tools (GNU's rm, mkdir and tar required)
CC=gcc
CP=cp
RM=rm
MKDIR=mkdir
TAR=tar
MSGFMT=msgfmt
XGETTEXT=xgettext

help:
	@echo "Available methods:"
	@echo "make $(PACKAGE) - just compile $(PACKAGE) (version: $(VERSION)),"
	@echo "make install - install this software in $(PREFIX) (defined in Makefile by PREFIX),"
	@echo "make clean - remove all created/compiled files,"
	@echo "make delete/make remove - remove files from $(PREFIX) (defined in Makefile by PREFIX),"
	@echo "make dist - create .tgz with sources - require GNU's tools."

$(PACKAGE):image.o raw.o atr.o fs.o atari_ii.o sparta.o interface.o franny.o
	g++ -s -L/lib -o $@ $^

fs.o: fs.cpp fs.h
	g++ -g -c -o $@ $<

atari_ii.o: atari_ii.cpp atari_ii.h fs.h
	g++ -g -c -o $@ $<

sparta.o: sparta.cpp sparta.h fs.h
	g++ -g -c -o $@ $<

atr.o: atr.cpp atr.h image.h
	g++ -g -c -o $@ $<

raw.o: raw.cpp raw.h image.h
	g++ -g -c -o $@ $<

image.o: image.cpp image.h
	g++ -g -c -o $@ $<

franny.o: franny.cpp atari_ii.h atr.h franny.h interface.h
	g++ -g -c -DVERSION=\"$(VERSION)\" -o $@ $<

interface.o: interface.cpp interface.h atr.h atari_ii.h sparta.h
	g++ -g -c -o $@ $<

clean:
	rm -f *.o $(PACKAGE)

install: $(PACKAGE) $(SCRIPTS) $(PACKAGE).mo $(PACKAGE).pot $(PACKAGE).1
	$(MKDIR) -p $(PREFIX)/bin
	$(CP) -f $(PACKAGE) $(PREFIX)/bin
	$(CP) -f $(SCRIPTS) $(PREFIX)/bin

remove:
	$(RM) -f $(PREFIX)/bin/$(PACKAGE)
	$(RM) -f $(PREFIX)/bin/$(SCRIPTS)

delete:
	$(RM) -f $(PREFIX)/bin/$(PACKAGE)
	$(RM) -f $(PREFIX)/bin/$(SCRIPTS)

dist:
	$(RM) -rf /tmp/$(PACKAGE)-$(VERSION)
	$(MKDIR) -p /tmp/$(PACKAGE)-$(VERSION)
	$(CP) -rf $(SCRIPTS) $(SOURCE_FILES) $(EXTRA_FILES) /tmp/$(PACKAGE)-$(VERSION)
	cd /tmp ; $(TAR) -zcf $(CURDIR)/$(PACKAGE)-$(VERSION).tgz $(PACKAGE)-$(VERSION)
	$(RM) -rf /tmp/$(PACKAGE)-$(VERSION)
