-include $(shell pwd)/Makefile.config

include ./Makefile.wx-config

ifeq (,$(shell which $(WXCONFIG)))
	DUMMY := $(error $(WXCONFIG) not found: Please ensure wxWidgets is correctly installed)
endif

ifeq (,$(wildcard crypto/cryptlib.cpp))
	DUMMY := $(error Crypto++ not found: Please read crypto/!readme.txt for details)
endif

.SUFFIXES: .o .cpp
.PRECIOUS: dirt
.PHONY: clean dirt all mac_post_link

WXPREFIX := $(shell $(WXCONFIG) --prefix)
WXVERSION := $(shell $(WXCONFIG) --version)
CC = g++
WINDRES = windres
REZ = $(shell $(WXCONFIG) --rezflags)
BUNDLE = Dirt.app/Contents

ifneq (,$(findstring wx_gtk2,$(WX_BASENAME)))
	CXXFLAGS_EXTRA = `pkg-config --cflags gtk+-2.0`
else
	ifneq (,$(findstring wx_gtk,$(WX_BASENAME)))
		CXXFLAGS_EXTRA = `gtk-config --cflags`
	endif
endif

ifneq (,$(findstring wx_mac,$(WX_BASENAME)))
	EXTRA_POST_LINK_CMD = @make mac_bundle
endif

ifneq (,$(findstring __WXDEBUG__,$(shell $(WXCONFIG) --cxxflags)))
	COMPILE_FLAGS = -g -DDEBUG
	STRIP = true||strip
else
	COMPILE_FLAGS = -O1 -DNDEBUG
	STRIP = strip
endif

CXXFLAGS = \
	$(strip \
		$(COMPILE_FLAGS) `$(WXCONFIG) --cxxflags` \
		$(CXXFLAGS_EXTRA) -I`$(WXCONFIG) --prefix`/include \
	)
LINK_FLAGS = `$(WXCONFIG) --libs`

SOURCES := $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o) crypto/libcryptopp.a
DIRT_EXE_PERMS = $(shell /bin/ls -l dirt | awk '{print $$1}' | tr -d "rw-")
DIRTCONSOLE_EXE_PERMS = $(shell /bin/ls -l dirtconsole | awk '{print $$1}' | tr -d "rw-")
DIRTSERVER_EXE_PERMS = $(shell /bin/ls -l dirtserver | awk '{print $$1}' | tr -d "rw-")

include ./Makefile.cross-compile

ifneq (,$(findstring wx_msw,$(WX_BASENAME)))
	OBJECTS := $(OBJECTS) Dirt.res
	EXTRA_CLEAN = Dirt.res
endif

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
	rm -f *.o Dirt$(BINARY_SUFFIX) DirtGTK.tar.bz2 $(EXTRA_CLEAN)
	rm -rf Dirt.app

all: clean dirt

.cpp.o : 
	$(CC) -c $(CXXFLAGS) -o $@ $<

%.d: %.cpp
	@set -e; $(CC) -MM $(CXXFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

-include $(SOURCES:.cpp=.d)

Dirt$(BINARY_SUFFIX): $(OBJECTS)
	$(CC) -o Dirt$(BINARY_SUFFIX) $(OBJECTS) $(LINK_FLAGS)
	$(STRIP) Dirt$(BINARY_SUFFIX)
	$(EXTRA_POST_LINK_CMD)

Dirt.res: Dirt.rc
	$(WINDRES) $(WINDRES_FLAGS) -i Dirt.rc -J rc -o Dirt.res -O coff

crypto/libcryptopp.a:
	@cd crypto && make

package: dirt
ifneq (,$(findstring wx_msw,$(WX_BASENAME)))
	upx --best Dirt$(BINARY_SUFFIX)
endif
ifneq (,$(findstring wx_gtk,$(WX_BASENAME)))
	test -f DirtGTK.tar.bz2 && rm DirtGTK.tar.bz2 || true
	tar cf DirtGTK.tar Dirt dirt dirtconsole
	cd res && tar rf ../DirtGTK.tar dirt.xpm
	bzip2 -9 DirtGTK.tar
endif
ifneq (,$(findstring wx_mac,$(WX_BASENAME)))
	test -f DirtMac.tar.bz2 && rm DirtMac.tar.bz2 || true
	tar cf DirtMac.tar Dirt.app
	bzip2 -9 DirtMac.tar
endif

mac_bundle: \
  $(BUNDLE)/MacOS \
  $(BUNDLE)/Resources/Dirt.rsrc \
  $(BUNDLE)/Resources/wxmac.icns \
  $(BUNDLE)/PkgInfo \
  $(BUNDLE)/Info.plist
	$(REZ) Dirt
	cp Dirt $(BUNDLE)/MacOS/Dirt

$(BUNDLE)/MacOS:
	install -d $@
    
$(BUNDLE)/Resources/Dirt.rsrc: $(WXPREFIX)/lib/libwx_mac-$(WXVERSION).rsrc
	@install -d `dirname $@`
	cp $< $@

$(BUNDLE)/Resources/wxmac.icns: res/dirt.icns
	@install -d `dirname $@`
	cp $< $@

$(BUNDLE)/PkgInfo:
	@install -d `dirname $@`
	echo -n "APPL????" > $@

$(BUNDLE)/Info.plist: res/Info.plist.in
	@install -d `dirname $@`
	sed -e "s/IDENTIFIER/`echo Dirt | sed 's,/,.,g'`/" \
	    -e "s/EXECUTABLE/Dirt/" \
	    -e "s/VERSION/3.0.0/" $< > $@
