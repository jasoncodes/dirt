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
3way.o      dh2.o       ida.o       polynomi.o  skipjack.o \
adler32.o   diamond.o   idea.o      pssr.o      socketft.o \
algebra.o   diamondt.o  integer.o   pubkey.o    square.o \
algparam.o  dsa.o       iterhash.o  queue.o     squaretb.o \
arc4.o      ec2n.o      luc.o       rabin.o     strciphr.o \
asn.o       eccrypto.o  mars.o      randpool.o  tea.o \
base64.o    ecp.o       marss.o     rc2.o       \
basecode.o  elgamal.o   md2.o       rc5.o       tftables.o \
              eprecomp.o  md4.o       rc6.o       tiger.o \
bfinit.o    esign.o     md5.o       rdtables.o  tigertab.o \
blowfish.o  files.o     md5mac.o    trdlocal.o \
blumshub.o  filters.o   misc.o      rijndael.o  twofish.o \
cast.o      fips140.o   modes.o     ripemd.o     \
casts.o     fipstest.o  modexppc.o  rng.o        \
channels.o  gf256.o     mqueue.o    rsa.o        \
crc.o       gf2_32.o    mqv.o       rw.o        wait.o \
cryptest.o  gf2n.o      nbtheory.o  safer.o     wake.o \
cryptlib.o  gfpcrypt.o  network.o   seal.o      winpipes.o \
  gost.o      oaep.o      serpent.o   xtr.o \
default.o   gzip.o      osrng.o     sha.o       xtrcrypt.o \
des.o       haval.o     panama.o    shark.o     zdeflate.o \
dessp.o     hex.o       pch.o       sharkbox.o  zinflate.o \
dh.o        hrtimer.o   pkcspad.o   simple.o    zlib.o

all: libcryptopp.a

clean:
	$(RM) libcryptopp.a $(LIBOBJS)

libcryptopp.a: $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)
	$(RANLIB) $@

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<
