lib_LTLIBRARIES = libpsi46interface.la

if HAVE_LIBFTDI
libpsi46interface_la_SOURCES = Delay.cc Log.cc USBInterface.libftdi.cc rpc.cpp rpc_error.cpp rpc_io.cpp

else
libpsi46interface_la_SOURCES = Delay.cc Log.cc USBInterface.libftd2xx.cc rpc.cpp rpc_error.cpp rpc_io.cpp
endif
libpsi46interface_la_CPPFLAGS = -I$(srcdir)/..

noinst_HEADERS = Delay.h Log.h USBInterface.h rpc.h rpc_error.h rpc_io.h 

