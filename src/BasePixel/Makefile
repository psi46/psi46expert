# define compile command

CC = g++

CFLAGS  = -Wall -g -m32
LDFLAGS =  -m32
SOFLAGS = -shared

# -- Default rule
$(addprefix ../obj/,%.o) : %.cc %.h
	$(CC) $(CFLAGS) -I../ -c $< -o $@


BASEPIXEL = TBInterface.o \
            TBAnalogInterface.o \
            SysCommand.o \
            DACParameters.o \
            TBParameters.o \
            TBAnalogParameters.o \
            ControlNetwork.o \
            Module.o \
            TBM.o \
            TBMParameters.o \
            ConfigParameters.o \
            Roc.o \
            DoubleColumn.o \
            Pixel.o \
            CalibrationTable.o \
            psi46_tb.o \
	    Keithley.o\
	    RawPacketDecoder.o \
	    DecoderCalibration.o \

lib: $(addprefix ../obj/,$(BASEPIXEL))
	$(CC) -shared -o ../lib/libBasePixel.so $(LDFLAGS) $(addprefix ../obj/,$(BASEPIXEL))

links:
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C0.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C1.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C2.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C3.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C4.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C5.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C6.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C7.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C8.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C9.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C10.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C11.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C12.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C13.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C14.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersModule/dacParameters_C15.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule/dacParameters_C0.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule/dacParameters_C1.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule/dacParameters_C2.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule/dacParameters_C3.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule/dacParameters_C4.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule/dacParameters_C5.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule/dacParameters_C6.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule/dacParameters_C7.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule2/dacParameters_C8.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule2/dacParameters_C9.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule2/dacParameters_C10.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule2/dacParameters_C11.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule2/dacParameters_C12.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule2/dacParameters_C13.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule2/dacParameters_C14.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersHalfModule2/dacParameters_C15.dat
	ln -sf ../defaultParametersRoc/dacParameters.dat defaultParametersRocNoTBM/dacParameters.dat
	
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C0.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C1.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C2.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C3.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C4.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C5.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C6.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C7.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C8.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C9.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C10.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C11.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C12.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C13.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C14.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersModule/trimParameters_C15.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule/trimParameters_C0.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule/trimParameters_C1.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule/trimParameters_C2.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule/trimParameters_C3.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule/trimParameters_C4.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule/trimParameters_C5.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule/trimParameters_C6.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule/trimParameters_C7.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule2/trimParameters_C8.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule2/trimParameters_C9.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule2/trimParameters_C10.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule2/trimParameters_C11.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule2/trimParameters_C12.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule2/trimParameters_C13.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule2/trimParameters_C14.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersHalfModule2/trimParameters_C15.dat
	ln -sf ../defaultParametersRoc/trimParameters.dat defaultParametersRocNoTBM/trimParameters.dat
	
	ln -sf ../defaultParametersRoc/testParameters.dat defaultParametersRocNoTBM/testParameters.dat
	ln -sf ../defaultParametersRoc/testParameters.dat defaultParametersModule/testParameters.dat
	ln -sf ../defaultParametersRoc/testParameters.dat defaultParametersHalfModule/testParameters.dat
	ln -sf ../defaultParametersRoc/testParameters.dat defaultParametersHalfModule2/testParameters.dat

	ln -sf ../defaultParametersRoc/tbParameters.dat defaultParametersRocNoTBM/tbParameters.dat
	ln -sf ../defaultParametersRoc/tbParameters.dat defaultParametersModule/tbParameters.dat
	ln -sf ../defaultParametersRoc/tbParameters.dat defaultParametersHalfModule/tbParameters.dat
	ln -sf ../defaultParametersRoc/tbParameters.dat defaultParametersHalfModule2/tbParameters.dat

	ln -sf ../defaultParametersRoc/tbmParameters.dat defaultParametersRocNoTBM/tbmParameters.dat
	ln -sf ../defaultParametersRoc/tbmParameters.dat defaultParametersModule/tbmParameters.dat

clean:
	rm *Dict*
	rm -f $(addprefix ../obj/,$(BASEPIXEL))
	rm -f core.*
	rm -f *~
	rm -f ../lib/libBasePixel.so
