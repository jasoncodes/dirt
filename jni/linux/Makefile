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

libs: libdirt_lib_ui_jfc.so libdirt_lib_util.so

libdirt_lib_ui_jfc.so: dirt_lib_ui_jfc_jni.c au_com_gslabs_dirt_lib_ui_jfc_jni_Linux.h
	$(CC) $(INCLUDES) $(LDFLAGS) -ldl -L$(X11LIB) -L$(JAVA_HOME)/jre/lib/i386 -ljawt -lXt -lX11 dirt_lib_ui_jfc_jni.c -o $@
	cp libdirt_lib_ui_jfc.so ../../lib/linux_x86/

libdirt_lib_util.so: dirt_lib_util_jni.c au_com_gslabs_dirt_lib_util_jni_Linux.h
	$(CC) $(INCLUDES) $(LDFLAGS) -ldl -L$(JAVA_HOME)/jre/lib/i386 dirt_lib_util_jni.c -o $@
	cp libdirt_lib_util.so ../../lib/linux_x86/

all: clean libs

clean:
	rm -f *.so *.o au_com_gslabs_dirt_jni_Linux.h au_com_gslabs_dirt_lib_util_jni_Linux.h

au_com_gslabs_dirt_lib_ui_jfc_jni_Linux.h: ../../build/main-classes/au/com/gslabs/dirt/lib/ui/jfc/jni/Linux.class
	javah -classpath ../../build/main-classes au.com.gslabs.dirt.lib.ui.jfc.jni.Linux

au_com_gslabs_dirt_lib_util_jni_Linux.h: ../../build/main-classes/au/com/gslabs/dirt/lib/util/jni/Linux.class
	javah -classpath ../../build/main-classes au.com.gslabs.dirt.lib.util.jni.Linux
