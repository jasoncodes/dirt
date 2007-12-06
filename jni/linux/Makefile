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

BUILD_UTIL = java -cp ../../build/main-classes au.com.gslabs.dirt.lib.util.BuildUtil

JAVA_HOME := $(shell $(BUILD_UTIL) --print-javahome)
ARCH := $(shell $(BUILD_UTIL) --print-arch)

INCLUDES := -I$(JAVA_HOME)/../include \
            -I$(JAVA_HOME)/../include/$(PLATFORM)

libs: libdirt_lib_ui_jfc.so libdirt_lib_util.so

/usr/include/X11/Intrinsic.h:
	@echo Install the libxt-dev package
	@false

libdirt_lib_ui_jfc.so: dirt_lib_ui_jfc_jni.c au_com_gslabs_dirt_lib_ui_jfc_jni_Linux.h /usr/include/X11/Intrinsic.h
	$(CC) $(INCLUDES) $(LDFLAGS) -ldl -L$(X11LIB) -L$(JAVA_HOME)/lib/$(ARCH) -ljawt -lXt -lX11 dirt_lib_ui_jfc_jni.c -o $@
	cp libdirt_lib_ui_jfc.so ../../lib/linux_$(ARCH)/

libdirt_lib_util.so: dirt_lib_util_jni.c au_com_gslabs_dirt_lib_util_jni_Linux.h
	$(CC) $(INCLUDES) $(LDFLAGS) -ldl -L$(JAVA_HOME)/lib/$(ARCH) dirt_lib_util_jni.c -o $@
	cp libdirt_lib_util.so ../../lib/linux_$(ARCH)/

all: clean libs

clean:
	rm -f *.so *.o au_com_gslabs_dirt_jni_Linux.h au_com_gslabs_dirt_lib_util_jni_Linux.h

au_com_gslabs_dirt_lib_ui_jfc_jni_Linux.h: ../../build/main-classes/au/com/gslabs/dirt/lib/ui/jfc/jni/Linux.class
	javah -classpath ../../build/main-classes au.com.gslabs.dirt.lib.ui.jfc.jni.Linux

au_com_gslabs_dirt_lib_util_jni_Linux.h: ../../build/main-classes/au/com/gslabs/dirt/lib/util/jni/Linux.class
	javah -classpath ../../build/main-classes au.com.gslabs.dirt.lib.util.jni.Linux

../../build/main-classes/au/com/gslabs/dirt/lib/ui/jfc/jni/Linux.class: ../../src/au/com/gslabs/dirt/lib/ui/jfc/jni/Linux.java
	cd ../.. && ant

../../build/main-classes/au/com/gslabs/dirt/lib/util/jni/Linux.class: ../../src/au/com/gslabs/dirt/lib/util/jni/Linux.java
	cd ../.. && ant
		
