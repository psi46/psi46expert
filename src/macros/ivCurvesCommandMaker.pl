#! /usr/bin/perl

# ----------------------------------------------------------------------
# Usage: ./ivSummary.pl M7861-24-1 etc.
#
# prints the command for ivCurve( -- 20 directories --) 
# into file ivCurvesCommmand.txt 
# ----------------------------------------------------------------------


my $root   = "root -l";
my $file   = "ivCurvesCommmand.txt";
  
  #open(PSI, "|$root -d $directory/$argnum ") || die "==> Cannot run root\n";
  open(PSI, ">".$file); 
  print PSI "ivCurves("; 
  # -- make all iv plots
  foreach $argnum (0 .. $#ARGV-1) {

    print PSI "\"$ARGV[$argnum]\","; 
  
  }
  
  print PSI "\"$ARGV[$#ARGV]\")";
  
  close(PSI);

