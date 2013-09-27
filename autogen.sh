#!/bin/bash

# Function that checks for program availability (works only
# with programs the accept the --version argument)
mkdir -p m4
function check_prog {
	# Try finding the program with the 'which' command.
	# Redirect any output to /dev/null.
	which $1 > /dev/null 2>&1
	
	# if it doesn't exist, you should get an error status
	if [[ $? != 0 ]]
	then
		echo "Error: program '$1' missing. Please install it."
		echo "Trying to continue ..."
	fi
}

# Check programs which are required for the next steps.
# This is for user friendliness only.
check_prog autoconf
check_prog automake
check_prog libtool

# Copy the original file to have the normal name
cp conf.ac.orig configure.ac

# Update the file if necessary
autoupdate

# Remove the copy the updating program leaves
rm -f configure.ac~

# Reconfigure
autoreconf --install
