ifneq ($(NOKDE),1)
	KAPP_H := $(shell kde-config --prefix)/include/kde/kapplication.h
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

CC = g++
CPPFLAGS = `wx-config --cxxflags` $(KDE_CPPFLAGS) -O3
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
	strip Dirt && upx --best Dirt && tar zcf DirtGTK.tar.gz Dirt dirt dirtconsole
