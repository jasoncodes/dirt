PROGRAM = Dirt

CC = gcc
.SUFFIXES:      .o .cpp
SOURCES := $(shell /bin/ls *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

.cpp.o :
	$(CC) -c `wx-config --cxxflags` -o $@ $<

$(PROGRAM): $(OBJECTS)
	$(CC) -o $(PROGRAM) $(OBJECTS) `wx-config --libs` crypto/libcryptopp.a
	chmod +x dirt dirtconsole

clean:
	rm -f *.o $(PROGRAM)

all:
	make clean
	make
