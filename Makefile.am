CC = gcc
.SUFFIXES:      .o .cpp
.SILENT: dirt
.PRECIOUS: dirt
.PHONY: clean
SOURCES := $(shell /bin/ls *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

.cpp.o :
	$(CC) -c `wx-config --cxxflags` -o $@ $<

dirt : Dirt
	chmod +x dirt dirtconsole

Dirt: $(OBJECTS)
	$(CC) -o Dirt $(OBJECTS) `wx-config --libs` crypto/libcryptopp.a

clean:
	rm -f *.o Dirt

all: clean dirt
