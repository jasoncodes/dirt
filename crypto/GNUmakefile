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

# overwrite CXXFLAGS with debug mode stuff if wx is debug version
ifneq (,$(findstring __WXDEBUG__,$(shell $(WXCONFIG) --cxxflags)))
	CXXFLAGS = -g -DDEBUG
endif

SRCS = $(wildcard *.cpp)
ifeq ($(SRCS),)				# workaround wildcard function bug in GNU Make 3.77
SRCS = $(shell ls *.cpp)
endif

LIBOBJS = \
3way.cpp      dh2.cpp       ida.cpp       polynomi.cpp  skipjack.cpp \
adler32.cpp   diamond.cpp   idea.cpp      pssr.cpp      socketft.cpp \
algebra.cpp   diamondt.cpp  integer.cpp   pubkey.cpp    square.cpp \
algparam.cpp  dsa.cpp       iterhash.cpp  queue.cpp     squaretb.cpp \
arc4.cpp      ec2n.cpp      luc.cpp       rabin.cpp     strciphr.cpp \
asn.cpp       eccrypto.cpp  mars.cpp      randpool.cpp  tea.cpp \
base64.cpp    ecp.cpp       marss.cpp     rc2.cpp       \
basecode.cpp  elgamal.cpp   md2.cpp       rc5.cpp       tftables.cpp \
              eprecomp.cpp  md4.cpp       rc6.cpp       tiger.cpp \
bfinit.cpp    esign.cpp     md5.cpp       rdtables.cpp  tigertab.cpp \
blowfish.cpp  files.cpp     md5mac.cpp    trdlocal.cpp \
blumshub.cpp  filters.cpp   misc.cpp      rijndael.cpp  twofish.cpp \
cast.cpp      fips140.cpp   modes.cpp     ripemd.cpp     \
casts.cpp     fipstest.cpp  modexppc.cpp  rng.cpp        \
channels.cpp  gf256.cpp     mqueue.cpp    rsa.cpp        \
crc.cpp       gf2_32.cpp    mqv.cpp       rw.cpp        wait.cpp \
cryptest.cpp  gf2n.cpp      nbtheory.cpp  safer.cpp     wake.cpp \
cryptlib.cpp  gfpcrypt.cpp  network.cpp   seal.cpp      winpipes.cpp \
  gost.cpp      oaep.cpp      serpent.cpp   xtr.cpp \
default.cpp   gzip.cpp      osrng.cpp     sha.cpp       xtrcrypt.cpp \
des.cpp       haval.cpp     panama.cpp    shark.cpp     zdeflate.cpp \
dessp.cpp     hex.cpp       pch.cpp       sharkbox.cpp  zinflate.cpp \
dh.cpp        hrtimer.cpp   pkcspad.cpp   simple.cpp    zlib.cpp

all: libcryptopp.a

clean:
	$(RM) libcryptopp.a $(LIBOBJS)

libcryptopp.a: $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	$(RANLIB) $@

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<
