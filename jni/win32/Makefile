# mingw32-gcc-3.4.2
#TARGET_ALIAS = mingw32
CC = i586-mingw32msvc-gcc
AS = i586-mingw32msvc-as
STRIP = i586-mingw32msvc-strip

INCLUDE = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/win32 -g
JAWT_LIB = $(JAVA_HOME)/lib/win32/jawt.lib
CFLAGS =  -g -O2 -c $(INCLUDE) $(LIB)

RM = rm -f

OBJ = dirt_lib_ui_jfc.o
BIN = dirt_lib_ui_jfc.dll

.PHONY: all all-before all-after clean clean-custom

all: all-before dirt_lib_ui_jfc.dll all-after

check: 
	echo $(JAVA_HOME)
clean: clean-custom
	${RM} $(OBJ) $(BIN) au_com_gslabs_dirt_lib_ui_jfc_jni_Win32.h

dirt_lib_ui_jfc.dll: dirt_lib_ui_jfc.o
	$(CC) -Wl,--add-stdcall-alias --shared -mno-cygwin -o dirt_lib_ui_jfc.dll dirt_lib_ui_jfc.o -s $(JAWT_LIB) -lunicows -luser32 -lsecur32 -lnetapi32
	$(STRIP) dirt_lib_ui_jfc.dll

dirt_lib_ui_jfc.o: dirt_lib_ui_jfc.c au_com_gslabs_dirt_lib_ui_jfc_jni_Win32.h
	$(CC) $(CFLAGS) dirt_lib_ui_jfc.c

au_com_gslabs_dirt_lib_ui_jfc_jni_Win32.h: ../../build/main-classes/au/com/gslabs/dirt/lib/ui/jfc/jni/Win32.class
	javah -classpath ../../build/main-classes au.com.gslabs.dirt.lib.ui.jfc.jni.Win32

../../build/main-classes/au/com/gslabs/dirt/lib/ui/jfc/jni/Win32.class: ../../src/au/com/gslabs/dirt/lib/ui/jfc/jni/Win32.java
	cd ../.. && ant

all-after:
	cp dirt_lib_ui_jfc.dll ../../lib/win32
