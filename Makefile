CC = gcc
CPPFLAGS = `wx-config --cxxflags`
.SUFFIXES:      .o .cpp
.PRECIOUS: dirt
.PHONY: clean
SOURCES := $(shell /bin/ls *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

dirt : Dirt
	@chmod +x dirt dirtconsole

.cpp.o : 
	$(CC) -c $(CPPFLAGS) -o $@ $<

%.d: %.cpp
	@set -e; $(CC) -MM $(CPPFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

include $(SOURCES:.cpp=.d)

Dirt: $(OBJECTS)
	$(CC) -o Dirt $(OBJECTS) `wx-config --libs` crypto/libcryptopp.a

clean:
	rm -f *.o *.d Dirt

all: clean dirt
