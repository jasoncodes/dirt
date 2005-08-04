# mingw32-gcc-3.4.2
#TARGET_ALIAS = mingw32
CC = i586-mingw32msvc-gcc
AS = i586-mingw32msvc-as
DLLWRAP = i586-mingw32msvc-dllwrap
DLLTOOL = i586-mingw32msvc-dlltool
DLLWRAP_FLAGS = --add-stdcall-alias --driver-name=i586-mingw32msvc-c++ -s $(JAVA_HOME)/lib/win32/jawt.lib

INCLUDE = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/win32 -g
LIB = -L$(JAVA_HOME)/lib/win32
CFLAGS =  -g -O2 -c $(INCLUDE) $(LIB)

DEFFILE=libjdic_misc.def
STATICLIB=libjdic_misc.a


RM = rm -f

OBJ = dirt_jni.o
BIN = dirt_jni.dll

.PHONY: all all-before all-after clean clean-custom

all: all-before dirt_jni.dll all-after

check: 
	echo $(JAVA_HOME)
clean: clean-custom
	${RM} $(OBJ) $(BIN)

dirt_jni.dll: dirt_jni.o
	$(DLLWRAP) $(DLLWRAP_FLAGS) -o dirt_jni.dll -s dirt_jni.o -def dirt_jni.def

dirt_jni.o: dirt_jni.c au_com_gslabs_dirt_jni_Win32.h
	$(CC) $(CFLAGS) dirt_jni.c

au_com_gslabs_dirt_jni_Win32.h: ../../build/main-classes/au/com/gslabs/dirt/jni/Win32.class
	javah -classpath ../../build/main-classes au.com.gslabs.dirt.jni.Win32
