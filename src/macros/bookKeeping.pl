#! /usr/bin/perl


# Usage: bookKeeping.pl -m M0090/T-10a
# -----

# ----------------------------------------------------------------------

print "\nbookKeeping> Starting ...\n";

use Getopt::Std;
getopts('m:d:rh');

$dbDir  = "/home/l_tester/ptr/moduleDB";

chdir("$dbDir");


@IN = split(/\//,$opt_m);

$modDirectory  = $IN[$#IN-1];
$subDir        = $IN[$#IN];

$modNumber    = substr($modDirectory,0,5);
$testNumber   = $subDir;


$entry = 0;
 
# Get overall & final Grade (if available)
$f = "$dbDir/$opt_m/summaryTest.txt";

$ovGrade = "NF";
$fnGrade = "TBD";

if (-e "$f" ) {
    
    open(GR, "$f") || die "Cannot open $f\n"; 
    
    while (<GR>) {
	if (/FINAL /) {
	    
	    s/FINAL DB-GRADE //g;
	    $fnGrade = $_;
	    chop($fnGrade);
	}
	if (/OVERALL /) {
	    
	    s/OVERALL GRADE //g;
	    $ovGrade = $_;
	    chop($ovGrade);
	}

	if (/shortTest /) {
	    
	    s/shortTest GRADE //g;
	    $ovGrade = $_;
	    chop($ovGrade);
	}
    }
    
    close(GR);
}
   
if ( $opt_r ) {
    
    $bookfile =  "book-keeping/retested-modules.dat";
}
elsif ( $opt_h ) { 
    
    $bookfile =  "book-keeping/half-module-tests.dat";
}
elsif ( $opt_d ) { 
    
    $bookfile =  "book-keeping/$opt_d.dat";
    if ( !(-e "$bookfile") ) { 

	system("/bin/touch $bookfile"); 
    }
}
else { 
    
    $bookfile =  "book-keeping/module-tests.dat";
}

open(IN, "$bookfile") || die "Cannot open $bookfile\n";

open(NEW,">$dbDir/bookTmp.txt") || die "Cannot open $dbDir/bookTmp.txt\n"; 

while (<IN>) {
    
    if (/$modNumber/) { 
	
	chop($_);
	$new = "$modNumber $fnGrade $modDirectory $ovGrade";
	
	if(!$entry) { 
	    
	    print NEW "$new\n";
	    print "bookKeeping> Updating the book-keeping before transfering data to kamor ...\n";

	    if ( $_ ne $new ) {
		print "bookKeeping> replacing \"$_\" with \"$new\"\n "; 
	    }
	    else {
		
		print "bookKeeping> No changes applied to $bookfile. (entry: $_)\n";
	    }
	}
	else { 
	    
	    print "bookKeeping> *** Warning: $modNumber was listed more than once in $bookfile !!!\n"; 
	}
	
	$entry = 1;
    }
    else {
	
	print NEW "$_";
    }
}

close(IN);
close(NEW);

system("mv $dbDir/bookTmp.txt $bookfile");

if ( !$entry ) {

    open(OUT, ">>$bookfile") || die "Cannot open $bookfile\n";
    $new = "$modNumber $fnGrade $modDirectory $ovGrade"; 
    print OUT "$new\n";
    print "bookKeeping> Wrote \"$new\" to $bookfile\n ";
    close(OUT);  
}
   

print "\nbookKeeping> ................................................ finished\n\n";
