-include $(shell pwd)/Makefile.config

ifneq ($(NOKDE),1)
	KAPP_H := $(shell kde-config --prefix 2> /dev/null)/include/kde/kapplication.h
	ifeq ($(wildcard $(KAPP_H)),$(KAPP_H))
		QAPP_H := $(QTDIR)/include/qapplication.h
		ifeq ($(wildcard $(QAPP_H)),$(QAPP_H))
			QTLIB := $(QTDIR)/lib/libqt-mt.so
			ifeq ($(wildcard $(QTLIB)),$(QTLIB))
				KDE_DIR := $(shell kde-config --prefix)
				KDE_CPPFLAGS := -I$(KDE_DIR)/include/kde -I$(QTDIR)/include -DKDE_AVAILABLE -DQT_THREAD_SUPPORT
				KDE_LINK := $(KDE_DIR)/lib/libkdeui.so $(QTDIR)/lib/libqt-mt.so
			endif
		endif
	endif
endif

BASENAME = $(shell wx-config --basename 2> /dev/null)
ifeq (,$(BASENAME))
	BASENAME = $(shell wx-config --libs | tr ' ' '\n' | grep wx_)
endif
ifneq (,$(findstring wx_gtk2,$(BASENAME)))
	GTK_EXTRAS = `pkg-config --cflags gtk+-2.0`
else
	ifneq (,$(findstring wx_gtk,$(BASENAME)))
		GTK_EXTRAS = `gtk-config --cflags`
	endif
endif

CC = g++
CPPFLAGS = $(strip -O1 `wx-config --cxxflags` $(KDE_CPPFLAGS) $(GTK_EXTRAS) -I`wx-config --prefix`/include) -DNDEBUG
.SUFFIXES: .o .cpp
.PRECIOUS: dirt
.PHONY: clean dirt all
SOURCES := $(shell /bin/ls *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DIRT_EXE_PERMS = $(shell /bin/ls -l dirt | awk '{print $$1}' | tr -d "rw-")
DIRTCONSOLE_EXE_PERMS = $(shell /bin/ls -l dirtconsole | awk '{print $$1}' | tr -d "rw-")
DIRTSERVER_EXE_PERMS = $(shell /bin/ls -l dirtserver | awk '{print $$1}' | tr -d "rw-")

dirt : Dirt
ifneq ($(DIRT_EXE_PERMS),xxx)
	chmod +x dirt
endif
ifneq ($(DIRTCONSOLE_EXE_PERMS),xxx)
	chmod +x dirtconsole
endif
ifneq ($(DIRTSERVER_EXE_PERMS),xxx)
	chmod +x dirtserver
endif

clean:
	rm -f *.o Dirt DirtGTK.tar.gz

all: clean dirt

.cpp.o : 
	$(CC) -c $(CPPFLAGS) -o $@ $<

%.d: %.cpp
	@set -e; $(CC) -MM $(CPPFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

-include $(SOURCES:.cpp=.d)

Dirt: $(OBJECTS) crypto/libcryptopp.a
	$(CC) -o Dirt $(OBJECTS) `wx-config --libs` crypto/libcryptopp.a $(KDE_LINK)

crypto/libcryptopp.a:
	@cd crypto && make

package: dirt
	strip Dirt && make tar

tar:
	test -f DirtGTK.tar.gz && rm DirtGTK.tar.gz || true
	tar cf DirtGTK.tar Dirt dirt dirtconsole
	cd res && tar rf ../DirtGTK.tar dirt.xpm
	gzip -9 DirtGTK.tar
