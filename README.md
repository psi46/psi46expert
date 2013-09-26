Build and installation of psi46expert for the digital testboard
===============================================================

1. Dependencies
---------------
To install psi46expert for the digital testboard a few libraries and programs are required:

  - libusb-1.0 and libusb-0.1
    normally available as a package for your Linux distribution
  - libftd2xx-1.1.x
    can be downloaded from http://www.ftdichip.com/Drivers/D2XX.htm
    Follow the instructions given on the website

  - ROOT
    download from http://root.cern.ch or try your Linux distriution's
    package repository
    export LD_LIBRARY_PATH=$HOME/root/lib:$LD_LIBRARY_PATH

    cd $HOME/root/lib
    ln -s /usr/lib/libusb.so libusb.so
  - libtool, autoconf, automake
    always available as a package for your Linux distribution,
    normally installed by default

If these are installed in special directories (not /usr or /usr/local)
then you have to take measures that the compiler finds your files
(set and export PATH, CPATH, LIBRARY_PATH environment variables).

2. Preparing the sources after SVN (subversion) checkout
--------------------------------------------------------
After checking out the sources from SVN, run the following command
in the working directory:

	./autogen.sh

3. Compiling the sources
------------------------
Run
	./configure [--prefix=SOMEDIR]
	make
or to use the ftd2xx libraries:
	./configure [--prefix=SOMEDIR] --with-ftd2xx
	make

with the option [--prefix=SOMEDIR] optional. The prefix determines
where the installed files would go. By default this is /usr/local
(binaries would go to /usr/local/bin, libraries to /usr/local/lib
and so on).

Multiple versions (e.g. branches) of the software can be installed
in parallel by changing the names of the produced programs by
running

	./configure [--prefix=SOMEDIR] --program-suffix=SUFFIX

Program names will then be for example psi46expertSUFFIX. Beware
that library names will not be affected by this (see below).

NOTE: Some linux versions, like RedHat Enterprise 6, 
have an old glibc version not compatible with libftd2xx-1.1.x

To use the software on such a linux system you can install GLIBC2.14
manually and use the following commands for compiling

    export LD_PRELOAD="/PATH/TO/GLIBC2.14/lib/libc.so.6:${LD_PRELOAD}"
    ./configure LDFLAGS="-L/PATH/TO/GLIBC2.14/lib"

5. Installing the software
--------------------------
The software is installed by typing

	make install

If multiple versions of the software should be installed in
parallel (see above) then the installation will overwrite
previously installed library versions (like libpsi46ana for
example). You can avoid this by specifying

	./configure <other args> --libdir=$PWD

which doesn't really install the libraries at all.

4. Running the software
-----------------------
You will need a configuration directory for Modules/ROCs:

	cp -r data/defaultParameters<X> WHEREVER

Then you can run

	psi46expert -dir WHEREVER

5. Frequent problems
--------------------
  0.    If you have an old test board, you need to make sure
        that the usb class can find it by using the method
	usb.SetProductID(0x6001) in the code
	(where 0x6001 corresponds to
	the product ID of the old FT chips that the previous
	boards were equipped with)
  1.	If ./autogen.sh does not work, you probably don't have
  	libtool installed. Or autoconf, automake. These are
  	standard packages in a Linux system. Check your Linux
  	manual on how to install packages.

  2.	If ./configure doesn't find some header or library,
	install what is requested and make sure it can be found
	by the compiler. The libraries libusb-1.0.so or libusb-0.1.so
	may require a symbolic link to libusb.so. This may also be
	true for libftd2xx. Make sure you follow the instructions of
	the libftd2xx library! libusb comes as a package in all Linux
	distributions. There are two versions: 0.1 and 1.0. Install
	both to be safe. (Your favourite distribution should provide
	the packages.)
	For libraries in special directories you can use

		export PATH=$PATH:whatever
		export CPATH=$CPATH:whatever
		export LIBRARY_PATH=$LIBRARY_PATH:whatever

	before calling ./configure . This is best put in your .bashrc
	file in your home directory.

  3.	If 'make' fails, you should talk to a code maintainer. There
  	are two cases which is known to fail: On is that the call to
  	rootcint fails because it cannot find the libraries. Make sure
  	that you set LD_LIBRARY_PATH correctly:

  		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:YOURPATH

  	The other case is if you try to compile
  	the software against the older version libftd2xx-0.4.x
  	on a 32 bit machine, then there may be a compilation error
  	in USBInterface.cc, depending on which compiler version you
  	are using. It might say something about being unable to
  	convert a 'unsigned long *' into a 'unsigned int *'. On a
  	32 bit Linux system, they are actually exactly the same, so
  	you can pass the variable

  		./configure CFLAGS=-fpermissive

  	to the configuration script, which should make the compiler
  	ignore the problem.

  4.	If psi46expert won't start because a library is missing,
  	set your custom library path with

  		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:YOURPATH

  5.	If psi46expert cannot open a port to the testboard, then
  	make sure the testboard name is correct in the configParameters.dat
  	file in you config directory. If this is the case, make
  	sure that you have write access to your USB devices:

  		ls -l /dev/bus/usb/00?/*

  	You can change these permissions permanently if you create a file
  	/etc/udev/rules.d/10-testboard.rules which contains

  		SUBSYSTEM == "usb", ATTR{product} == "DLP-USB245M", GROUP="usb", MODE="0664"

  	and create a group "usb" and add yourself to that group. Newly
  	plugged devices will have the correct permissions. For the
  	libftd2xx-0.4.x library version it may be that you need the
  	usbfs. You can get this by putting the following line into your
  	/etc/fstab file:

  		none /proc/bus/usb usbfs devgid=1000,devmode=0660 0 0

  	where you should replace the number 1000 with the group id of
  	your group 'usb'. You can then either reboot the machine or
  	do a

  		mount -a

  	as root.
  	Lastly, it is possible that the automatic detachment of the
  	usb serial kernel modules was not enabled during compilation.
  	This happens if the libusb-1.0 headers are missing on the
  	system. Some systems have -devel packages for their libraries
  	and you should install such package and then recompile and
  	reinstall psi46expert.

  6.	If you wish to use other devices with FTDI chips that require
  	the bit bang mode and/or the kernel modules usbserial and
  	ftdi_sio, then you should install libftd2xx-1.1.x.

  7. Some linux versions, like RedHat Enterprise 6, 
     have an old glibc version not compatible with libftd2xx-1.1.x

     To use the software on such a linux system you can install GLIBC2.14
     manually and use the following commands for compiling

     export LD_PRELOAD="/PATH/TO/GLIBC2.14/lib/libc.so.6:${LD_PRELOAD}"
     ./configure LDFLAGS="-L/PATH/TO/GLIBC2.14/lib"
 
  8.	If all things fail, you should talk to a code maintainer. Post
  	a message to the hypernews list 'Pixel psi46 Testboard':

  		hn-cms-pixel-psi46-testboard@cern.ch

  	This requires a CERN account.
