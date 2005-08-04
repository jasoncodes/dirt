# mingw32-gcc-3.4.2
#TARGET_ALIAS = mingw32
CC = i586-mingw32msvc-gcc
AS = i586-mingw32msvc-as
DLLWRAP = i586-mingw32msvc-dllwrap
DLLTOOL = i586-mingw32msvc-dlltool
DLLWRAP_FLAGS = --add-stdcall-alias --driver-name=i586-mingw32msvc-c++

INCLUDE = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/win32 -g
DLLWRAP_LIB = -s $(JAVA_HOME)/lib/win32/jawt.lib
CFLAGS =  -g -O2 -c $(INCLUDE) $(LIB)

RM = rm -f

OBJ = dirt_jni.o
BIN = dirt_jni.dll

.PHONY: all all-before all-after clean clean-custom

all: all-before dirt_jni.dll all-after

check: 
	echo $(JAVA_HOME)
clean: clean-custom
	${RM} $(OBJ) $(BIN) au_com_gslabs_dirt_jni_Win32.h

dirt_jni.dll: dirt_jni.o
	$(DLLWRAP) $(DLLWRAP_FLAGS) -o dirt_jni.dll -s dirt_jni.o $(DLLWRAP_LIB)

dirt_jni.o: dirt_jni.c au_com_gslabs_dirt_jni_Win32.h
	$(CC) $(CFLAGS) dirt_jni.c

au_com_gslabs_dirt_jni_Win32.h: ../../build/main-classes/au/com/gslabs/dirt/jni/Win32.class
	javah -classpath ../../build/main-classes au.com.gslabs.dirt.jni.Win32

all-after:
	cp dirt_jni.dll ../../lib/win32
