# these flags are overwritten below if wx is in debug mode
CXXFLAGS = -O1 -DNDEBUG
LDFLAGS =
ARFLAGS = -cr	# ar needs the dash on OpenBSD
RANLIB = ranlib
UNAME = $(shell uname)

ifeq ($(UNAME),)	# for DJGPP, where uname doesn't exist
CXXFLAGS += -mbnu210
else
CXXFLAGS += -pipe
endif

ifeq ($(UNAME),Darwin)
AR = libtool
ARFLAGS = -static -o
CXXFLAGS += -D__pic__
IS_GCC2 = $(shell c++ -v 2>&1 | grep -c gcc-932)
ifeq ($(IS_GCC2),1)
CXXFLAGS += -fno-coalesce-templates -fno-coalesce-static-vtables
CXX = c++
LDLIBS += -lstdc++
LDFLAGS += -flat_namespace -undefined suppress -m
endif
endif

ifeq ($(UNAME),SunOS)
LDLIBS += -lnsl -lsocket
endif

ifeq ($(CXX),gcc)	# for some reason CXX is gcc on cygwin 1.1.4
CXX = g++
endif

-include ../Makefile.config
include ../Makefile.wx-config
include ../Makefile.cross-compile

# the gcc that comes with OSX seems to die hard on optimizations
ifneq (,$(findstring wx_mac,$(WX_BASENAME)))
	CXXFLAGS := $(subst -O1,,$(CXXFLAGS))
endif

# overwrite CXXFLAGS with debug mode stuff if wx is debug version
ifneq (,$(findstring __WXDEBUG__,$(shell $(WXCONFIG) --cxxflags)))
	CXXFLAGS = -g -DDEBUG
endif

SRCS = $(wildcard *.cpp)
ifeq ($(SRCS),)				# workaround wildcard function bug in GNU Make 3.77
SRCS = $(shell ls *.cpp)
endif

LIBOBJS = \
	adler32.o asn.o algparam.o base64.o basecode.o crc.o cryptlib.o \
	des.o dessp.o filters.o fips140.o integer.o iterhash.o md5.o \
	md5mac.o misc.o modes.o mqueue.o nbtheory.o osrng.o pkcspad.o \
	pubkey.o queue.o randpool.o rdtables.o rijndael.o rsa.o sha.o \
	zdeflate.o zinflate.o zlib.o pssr.o

all: libcryptopp.a

clean:
	$(RM) libcryptopp.a $(LIBOBJS)

libcryptopp.a: $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	$(RANLIB) $@

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<
