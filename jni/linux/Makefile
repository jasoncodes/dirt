UNAME = $(shell uname)

ifeq ($(UNAME), SunOS)
  PLATFORM = solaris
  CXX = CC
  CC = cc  
  CXXFLAGS = -c
  LDFLAGS = -G -ldl -lrt -lpthread
  X11LIB=/usr/openwin/lib
else 
  ifeq ($(UNAME), Linux)
    PLATFORM = linux
    CXX = g++
    CXXFLAGS = -c
    LDFLAGS = -shared -fPIC -ldl -lrt -lpthread
  endif
  ifeq ($(UNAME), FreeBSD)
    PLATFORM = freebsd
    CXX = g++
    CXXFLAGS = -c -I/usr/local/include -I/usr/X11R6/include
    LDFLAGS = -shared -fPIC -L/usr/local/lib -L/usr/X11R6/lib
  endif
  CC = gcc
  X11LIB=/usr/X11R6/lib
endif

USR_INCLUDE_DIR = /usr/include
USR_LIB_DIR = /usr/lib

INCLUDES = -I$(JAVA_HOME)/include \
           -I$(JAVA_HOME)/include/$(PLATFORM)

SRC = dirt_jni.c
          
SO_NAME = libdirt_jni.so

SO_OBJS = $(CPPSRCS:.cpp=.o)

$(SO_NAME): $(SRC) au_com_gslabs_dirt_jni_Linux.h
	$(CC) $(INCLUDES) $(LDFLAGS) -ldl -L$(X11LIB) -L$(JAVA_HOME)/jre/lib/i386 -ljawt -lXt -lX11 $(SRC) -o $@
	cp libdirt_jni.so ../../lib/linux_x86/

all: clean $(SO_NAME)

clean:
	rm -f *.so *.o au_com_gslabs_dirt_jni_Linux.h

au_com_gslabs_dirt_jni_Linux.h: ../../build/main-classes/au/com/gslabs/dirt/jni/Linux.class
	javah -classpath ../../build/main-classes au.com.gslabs.dirt.jni.Linux
