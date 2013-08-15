#! /usr/bin/perl


# Usage: scanDBgrades.pl -m M7690-17-2 
# -----

# ----------------------------------------------------------------------
use Getopt::Std;
getopts('m:');

$dbDir   = "/home/l_tester/ptr/moduleDB";

@IN = split(/\//,$opt_m);

$mainD = $IN[$#IN-1];
$subD  = $IN[$#IN];

($moduleNr, $nada) = split(/\-/, $mainD);
$testNr            = $subD;
 
$fnGrade = "TBD";

if (-e "$dbDir/$opt_m/../finalGrade.txt" ) { system("rm -f $dbDir/$opt_m/../finalGrade.txt"); }
open(GRADE, ">$dbDir/$opt_m/../finalGrade.txt") || die "==> Cannot open $dbDir/$opt_m/../finalGrade.txt\n";

$fileM = "$dbDir/book-keeping/module-tests.dat";
$fileH = "$dbDir/book-keeping/half-module-tests.dat";

if ( -e $fileM && -e $fileH  ) {

    getFnGrade($fileM);
    getFnGrade($fileH);

} else {
    print "\tscanDBgrades> COULD NOT FIND $fileM \/ $fileH\n";
}

if ( $fnGrade ne "A" && $fnGrade ne "B" && $fnGrade ne "C"  ) {
    
    # for module production: do not generate final grade autom.
    $fnGrade = "TBD";
}

print GRADE "FINAL DB-GRADE $fnGrade\n";
close(GRADE);

print "\tscanDBgrades> Wrote grade \"FINAL DB-GRADE $fnGrade\" to $mainD/finalGrade.txt\n";
#----------------------------------------------------------------------------------------

sub getFnGrade() {
    my ($f) = @_;

    open(GR, "$f") || die "Cannot open $f\n"; 

    print "\tscanDBgrades> LOOKING FOR $moduleNr\'s grade\n";
    while (<GR>) {
	if (/$moduleNr/) {

	    ($mn, $fnGrade, $ba, $lu, $dn, $ovGrade) = split(/\ /,$_);
	
	    if ( $fnGrade ne "" ) {
		print "\tscanDBgrades> FOUND FINAL GRADE for $mn IN DB:  $fnGrade\n";
	    }
	}
    }
}
