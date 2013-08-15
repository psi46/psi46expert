# define compile command

CC = g++

CVS = cvs

CFLAGS  = -Wall -g -Wno-deprecated 
LDFLAGS = -g -lusb
LDFLAGS = -L /usr/local/lib -lusb --allow-shlib-undefined
LDFLAGS = -L /usr/local/lib -lusb -lSpectrum
SOFLAGS = -shared -g

ROOTCFLAGS    = $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS      = $(shell $(ROOTSYS)/bin/root-config --libs)
ROOTGLIBS     = $(shell $(ROOTSYS)/bin/root-config --glibs)

CFLAGS       += $(ROOTCFLAGS)

OBJECTS=BinaryFileReader.o Viewer.o ViewerDict.o PHCalibration.o ConfigReader.o\
	 LangauFitter.o RocGeometry.o
TOBJECTS=BinaryFileReader.o Viewer.o ViewerDict.o PHCalibration.o\
	 LangauFitter.o EventReader.o ConfigReader.o Plane.o\
	 RocGeometry.o EventView.o

.cc.o:
	$(CC) $(CFLAGS) -c $<

r: r.cxx $(OBJECTS)
	$(CC) $(CFLAGS) -I $(CVS) $(LDFLAGS) $(ROOTGLIBS) r.cxx -o r \
	$(OBJECTS)

r2: r2.cxx $(OBJECTS)
	$(CC) $(CFLAGS) -I $(CVS) $(LDFLAGS) $(ROOTGLIBS) r2.cxx -o r2 \
	$(OBJECTS)

t: t.cxx $(TOBJECTS) 
	$(CC) $(CFLAGS) -I $(CVS) $(LDFLAGS) $(ROOTGLIBS) t.cxx -o t \
	$(TOBJECTS)

raw: raw.cxx
	$(CC) raw.cxx -o raw

gen: gen.cxx RocGeometry.o Plane.o ConfigReader.o
	$(CC) $(CFLAGS) gen.cxx RocGeometry.o Plane.o ConfigReader.o -o gen

ViewerDict.cc: Viewer.h ViewerLinkDef.h
	$(ROOTSYS)/bin/rootcint  -f ViewerDict.cc -c Viewer.h ViewerLinkDef.h


clean:	
	rm *.o
	rm ViewerDict.cc ViewerDict.h

