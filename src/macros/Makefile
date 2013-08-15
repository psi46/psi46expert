# define compile command
  
CC = g++

CFLAGS  = -Wall -o2 -Wno-deprecated
#LDFLAGS = -lusb
#LDFLAGS = -L /usr/local/lib -lusb --allow-shlib-undefined
#LDFLAGS = -L /usr/local/lib -lusb 
SOFLAGS = -shared


ROOTCFLAGS    = $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS      = $(shell $(ROOTSYS)/bin/root-config --libs)
ROOTGLIBS     = $(shell $(ROOTSYS)/bin/root-config --glibs)
GLIBS         = $(filter-out -lz, $(ROOTGLIBS))

CFLAGS       += $(ROOTCFLAGS)



# ----------------------------------------------------------------------
pixelVariationLR: 
	$(CC) -g $(CFLAGS) -I../ $(LDFLAGS) pixelVariationLowRange.C -o ../bin/pixelVariationLowRange $(ROOTGLIBS)  
pixelVariationHR: 
	$(CC) -g $(CFLAGS) -I../ $(LDFLAGS) pixelVariationHighRange.C -o ../bin/pixelVariationHighRange $(ROOTGLIBS)  

# ----------------------------------------------------------------------
ntplAna: 
	$(CC) $(CFLAGS) -I../ $(LDFLAGS) ntplAna.C -o ../bin/ntplAna $(ROOTGLIBS)  



# ----------------------------------------------------------------------
ntpTestResults: ntpTestResults.cc 
# -------------------------------
	$(CC) $(CFLAGS)  -c ntpTestResults.cc -o ntpTestResults.o
	$(CC) $(LDFLAGS) -o ntpTestResults ntpTestResults.o $(GLIBS)


# ----------------------------------------------------------------------
runAnaTestResults: anaTestResults.cc runAnaTestResults.cc 
# -------------------------------------------------------
	$(CC) $(CFLAGS) -c anaTestResults.cc    -o anaTestResults.o
	$(CC) $(CFLAGS) -c runAnaTestResults.cc -o runAnaTestResults.o
	$(CC) $(LDFLAGS)  -o runAnaTestResults runAnaTestResults.o anaTestResults.o $(GLIBS)


myAnaTestResults: myAnaTestResults.cc myRunAnaTestResults.cc
# -------------------------------------------------------
	$(CC) $(CFLAGS) -c myAnaTestResults.cc    -o myAnaTestResults.o
	$(CC) $(CFLAGS) -c myRunAnaTestResults.cc -o myRunAnaTestResults.o
	$(CC) $(LDFLAGS)  -o myRunAnaTestResults myRunAnaTestResults.o myAnaTestResults.o $(GLIBS)


myReproTestResults: myReproTestResults.cc myRunReproTestResults.cc
# -------------------------------------------------------
	$(CC) $(CFLAGS) -c myReproTestResults.cc    -o myReproTestResults.o
	$(CC) $(CFLAGS) -c myRunReproTestResults.cc -o myRunReproTestResults.o
	$(CC) $(LDFLAGS)  -o myRunReproTestResults myRunReproTestResults.o myReproTestResults.o $(GLIBS)


combinedAnaTestResults: myAnaTestResults.cc combinedAnaTestResults.cc
# -------------------------------------------------------
	$(CC) $(CFLAGS) -c myAnaTestResults.cc    -o myAnaTestResults.o
	$(CC) $(CFLAGS) -c combinedAnaTestResults.cc -o combinedAnaTestResults.o
	$(CC) $(LDFLAGS)  -o combinedAnaTestResults combinedAnaTestResults.o myAnaTestResults.o $(GLIBS)


# ----------------------------------------------------------------------
clean:
	rm -f *Dict.*  *.o
	rm -f runAnaTestResults 
	rm -f ntpTestResults
	rm -f myAnaTestResults
	rm -f myReproTestResults
	rm -f combinedAnaTestResults

