-include $(shell pwd)/Makefile.config

include ./Makefile.wx-config

ifneq (,$(findstring wx_gtk2,$(WX_BASENAME)))
	GTK_EXTRAS = `pkg-config --cflags gtk+-2.0`
else
	ifneq (,$(findstring wx_gtk,$(WX_BASENAME)))
		GTK_EXTRAS = `gtk-config --cflags`
	endif
endif

CC = g++
STRIP = strip
WINDRES = windres
OPTIMIZATIONS = -O1

CXXFLAGS = \
	$(strip \
		$(OPTIMIZATION) `$(WXCONFIG) --cxxflags` \
		$(GTK_EXTRAS) -I`$(WXCONFIG) --prefix`/include -DNDEBUG \
	)
LINK_FLAGS = `$(WXCONFIG) --libs`
.SUFFIXES: .o .cpp
.PRECIOUS: dirt
.PHONY: clean dirt all
SOURCES := $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o) crypto/libcryptopp.a
DIRT_EXE_PERMS = $(shell /bin/ls -l dirt | awk '{print $$1}' | tr -d "rw-")
DIRTCONSOLE_EXE_PERMS = $(shell /bin/ls -l dirtconsole | awk '{print $$1}' | tr -d "rw-")
DIRTSERVER_EXE_PERMS = $(shell /bin/ls -l dirtserver | awk '{print $$1}' | tr -d "rw-")

include ./Makefile.cross-compile

dirt : Dirt$(BINARY_SUFFIX)
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
	$(CC) -c $(CXXFLAGS) -o $@ $<

%.d: %.cpp
	@set -e; $(CC) -MM $(CXXFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

-include $(SOURCES:.cpp=.d)

Dirt$(BINARY_SUFFIX): $(OBJECTS) $(EXTRA_DEPENDS)
	$(CC) -o Dirt$(BINARY_SUFFIX) $(OBJECTS) $(LINK_FLAGS) $(EXTRA_DEPENDS)
	$(STRIP) Dirt$(BINARY_SUFFIX)

Dirt.res: Dirt.rc
	$(WINDRES) \
	$(subst -I,--include-dir=,\
		$(subst $(OPTIMIZATIONS),,\
			$(CXXFLAGS)\
		) \
	)\
	-i Dirt.rc -J rc -o Dirt.res -O coff

crypto/libcryptopp.a:
	@cd crypto && make

package: dirt
	test -f DirtGTK.tar.gz && rm DirtGTK.tar.gz || true
	tar cf DirtGTK.tar Dirt dirt dirtconsole
	cd res && tar rf ../DirtGTK.tar dirt.xpm
	gzip -9 DirtGTK.tar
