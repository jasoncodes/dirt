ifneq ($(wildcard $(shell kde-config --prefix)/include/kde/kapplication.h),.)
ifneq ($(wildcard $(QTDIR)/include/qapplication.h),.)
KDE_CPPFLAGS := -I`kde-config --prefix`/include/kde -I$(QTDIR)/include -DKDE_AVAILABLE
KDE_LINK := `kde-config --prefix`/lib/libkdeui.so $(QTDIR)/lib/libqt-mt.so
endif
endif

CC = gcc
CPPFLAGS = `wx-config --cxxflags` $(KDE_CPPFLAGS)
.SUFFIXES: .o .cpp
.PRECIOUS: dirt
.PHONY: clean dirt all
SOURCES := $(shell /bin/ls *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DIRT_EXE_PERMS = $(shell /bin/ls -l dirt | awk '{print $$1}' | tr -d "rw-")
DIRTCONSOLE_EXE_PERMS = $(shell /bin/ls -l dirtconsole | awk '{print $$1}' | tr -d "rw-")

dirt : Dirt
ifneq ($(DIRT_EXE_PERMS),xxx)
	chmod +x dirt
endif
ifneq ($(DIRTCONSOLE_EXE_PERMS),xxx)
	chmod +x dirtconsole
endif

clean:
	rm -f *.o Dirt

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
