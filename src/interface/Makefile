# define compile command

CC = g++

#ifneq (,$(PSI46CC))
#CC = g++
#endif

CFLAGS  =  -Wall -g -m32
LDFLAGS = -m32 -lftd2xx -lusb

# -- Default rule
$(addprefix ../obj/,%.o) : %.cc
	$(CC) $(CFLAGS) -I../ -c $< -o $@


INTERFACE = USBInterface.o \
            Delay.o \
	    Log.o
	     
lib: $(addprefix ../obj/,$(INTERFACE))
	$(CC) $(LDFLAGS) -shared -o ../lib/libinterface.so $(addprefix ../obj/,$(INTERFACE))

clean:
	rm *Dict*
	rm -f $(addprefix ../obj/,$(INTERFACE))
	rm -f core.*
	rm -f *~
	rm -f  ../lib/libinterface.so
