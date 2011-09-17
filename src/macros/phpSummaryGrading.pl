#! /usr/bin/perl


# Usage: phpSummaryGrading.pl -m M0090/T-10a
# -----

# ----------------------------------------------------------------------
print "\nphpSummaryGrading> Starting ...\n";

use Getopt::Std;
getopts('m:rhz');

$dbDir   = "/home/l_tester/ptr/moduleDB";

@IN = split(/\//,$opt_m);

$mainD = $IN[$#IN-1];
$subD  = $IN[$#IN];

$moduleNr  = substr($mainD,0,5);
$testNr    = $subD;

my @temp;
my @grade;
my @i150V;

my $ok;

$#temp = -1;
$#grade = -1;
$#i150V = -1;

push(@temp, "T-10a");
push(@temp, "T-10b");
push(@temp, "T+17a");

# --- some grading information per chip -----------------------------------------------------------------

if ( (-e  "$dbDir/$opt_m/gradingTest.txt") && (-e  "$dbDir/$opt_m/summaryTest.txt") ) {
    system("/bin/cat $dbDir/$opt_m/gradingTest.txt >> $dbDir/$opt_m/summaryTest.txt");
}
else {
    print "phpSummaryGrading> !!!!!!!!!  ----> COULD NOT FIND ONE OF THE FOLLOWING FILES: \n $dbDir/$opt_m/gradingTest.txt \n $dbDir/$opt_m/summaryTest.txt\n";
}

if ( (-e  "$dbDir/$opt_m/gradingTest.txt") ) {
    system("rm -f $dbDir/$opt_m/gradingTest.txt");
}

# --- START ----------------------------------------------------------------------------------------------


open(OVERALL, ">>$dbDir/$opt_m/summaryTest.txt") || die "==> Cannot open $dbDir/$opt_m/summaryTest.txt\n";

foreach $dir ( @temp ) {
    
    $file = "$dbDir/$opt_m/../$dir/summaryTest.txt";
    if(-e $file) { getGrades($file); }
    
}

# --- ratio of recalcuated and measured current at 17 C --------------------------------------------------
print "phpSummaryGrading> $opt_m\n";

if ( @i150V[2]!=0 ) { $ratio = @i150V[1]/@i150V[2]; }

if ( $ratio ) { 

    print "phpSummaryGrading> 17/10 RATIO   $ratio\n"; 
    print OVERALL "RATIO $ratio\n";
}
else { 

    print "phpSummaryGrading> 17/10 RATIO   N/A\n"; 
    print OVERALL "RATIO 0\n";
}


# --- Overall Grading ------------------------------------------------------------------------------------

$tmpGrade = "A";

if ( $#grade == 2 || $opt_z ) {

    foreach $i (0 .. $#grade) {
	
	chop($grade[$i]);
	print "phpSummaryGrading> @temp[$i] GRADE     ... $grade[$i]\n";
	
	if ( $grade[$i] eq "C" ) { $tmpGrade = "C"; last; }
	if ( $overG[$i] eq "C" ) { $tmpGrade = "C"; last; }
	if ( $grade[$i] eq "B" ) { $tmpGrade = "B"; next; }
	if ( $overG[$i] eq "B" ) { $tmpGrade = "B"; next; }
    }

} else {
    
    $tmpGrade = "C";
}

if ( $opt_z ) {

    print OVERALL "shortTest GRADE $tmpGrade\n"; 

}  else {

    print OVERALL "OVERALL GRADE $tmpGrade\n"; 
}

print "phpSummaryGrading> OVERALL GRADE   ... $tmpGrade\n";
print "phpSummaryGrading> ---> written to $dbDir/$opt_m/summaryTest.txt ...\n";
close(OVERALL);

print "\nphpSummaryGrading> Starting automatic grading ...\n";

# ------ Automatic final grade -------------------------------------------

if ( $opt_r || $opt_z ) { 
    
    print "\n\t %%%%%%%% scanning DB grades for $moduleNr %%%%%%%% \n\n";
    system("./scanDBgrades.pl -m $opt_m");
}
elsif ( $opt_h ) {

    #print "********************** ./gradeModule.pl -h -m $opt_m -G $tmpGrade\n";
    system("./gradeModule.pl -h -m $opt_m -G $tmpGrade");
}
else {

    #print "********************** ./gradeModule.pl -m $opt_m -G $tmpGrade\n";
    system("./gradeModule.pl -m $opt_m -G $tmpGrade");
}

# ------------------------------------------------------------------------

if ( -e "$dbDir/$opt_m/../finalGrade.txt" ) {
    
    system("/bin/cat $dbDir/$opt_m/../finalGrade.txt >> $dbDir/$opt_m/summaryTest.txt");
}
else {
    
    print "phpSummaryGrading> COULD NOT FIND FINAL GRADE $dbDir/$opt_m/../finalGrade.txt\n";
}


print "\nphpSummaryGrading> ................................................ finished\n\n";
#---------------------------------------------------------------------------------------------
sub getGrades() {
    my ($f) = @_;
    
    open(IN, "$f") || die "Cannot open $file\n";
    while (<IN>) {
	
	if (/Grade/) {
	    s/Grade //g; 
	    push(@grade, $_);
	}	

	if (/OVERALL/) {
	    s/OVERALL GRADE //g; 
	    push(@overG, $_);
	}	

	if (/shortTest/) {
	    s/shortTest GRADE //g; 
	    push(@overG, $_);
	}

	if (/I\ 150/) {
	    s/I\ 150 //g;
	    push(@i150V, $_);
	}
	
    }
}
