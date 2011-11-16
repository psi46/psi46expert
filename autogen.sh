#!/bin/bash

# Copy the original file to have the normal name
cp conf.ac.orig configure.ac

# Update the file if necessary
autoupdate

# Remove the copy the updating program leaves
rm -f configure.ac~

# Reconfigure
autoreconf --install
