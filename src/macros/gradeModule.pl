#! /usr/bin/perl


# Usage: gradeModule.pl -m M7690-17-2 -G A -F "explanation"
# -----

# ONLY FOR MODULE TESTS (NOT TO BE USED FOR RETESTED MODULES, IRR. MODULES ETC. !!!

# ----------------------------------------------------------------------
use Getopt::Std;
getopts('m:G:F:h');

$dbDir   = "/home/l_tester/ptr/moduleDB";

print "\n";

chdir("$dbDir");

$modNumber    = substr($opt_m,0,5);
($modDirectory, $subDir) = split(/\//, $opt_m);

$#dir = -1;
$entry = 0;

@dir    = `ls -d $modNumber*`;
@subdir = `ls -d $modDirectory/T?1??`;

open(GRAD, ">tempFinalGrade.txt") || die "==> Cannot open $tempFinalGrade.txt\n";

# Get overall & final Grade (not needed!)
chop(@subdir[0]);
$f = "$dbDir/@subdir[0]/summaryTest.txt";
$g = "$dbDir/$opt_m/comment_2.txt";

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

#--------- Write final grade to finalGrade.txt and to book-keeping file --------------------------
if ($opt_G) {
    
    # -- automatic grading only if the module was not graded manually before
    if ( !$opt_F && -e "$g") {

	print  "\n  gradeModule> ***** Module $modNumber was graded manually to $fnGrade *****\n";
	print  "  gradeModule> ***** Therefore it will not be graded automatically anymore *****\n\n";
	exit;
    }

    # -- automatic grade
    if (  $fnGrade && ($fnGrade ne $opt_G) ) {

	print  "\n  gradeModule> ***** Module $modNumber will be (re)graded from $fnGrade ---> $opt_G *****\n\n";
    }

    # -- manual grade	
    if (  $opt_F && ($ovGrade ne $opt_G) ) {
	    
	print  "\n  gradeModule> ***** You have chosen a grade ($opt_G) different from the overall grade ($ovGrade)\n";
	print  "  gradeModule> ***** Your comment: \"$opt_F\"\n\n";	
    }	
    

    # ============== Start updating all the files with new Grade ==================================

    # -- book-keeping file 
    if ( $opt_h ) {

	$bookfile = "$dbDir/book-keeping/half-module-tests.dat";
    
    } else {

	$bookfile = "$dbDir/book-keeping/module-tests.dat";
    }

    open(IN, "$bookfile") || die "Cannot open $bookfile\n"; 
    open(NEW,">$dbDir/gradesTmp.txt") || die "Cannot open $dbDir/gradesTmp.txt\n"; 

    while (<IN>) {
	if (/$modNumber/) { 

	    chop($_);
	    $new = "$modNumber $opt_G $modDirectory $ovGrade";

	    if(!$entry) { 

		print NEW "$new\n"; 
		print "  gradeModule> Updating the book-keeping ...\n";
		if ( $_ ne $new ) {

		    print "  gradeModule> replacing \"$_\" \n \t\t  with \"$new\"\n ";
		}
		else {
		    
		    print "  gradeModule> No changes applied to $bookfile.\n";
		}
	    }
	    else { 

		print "  gradeModule> $modNumber was listed more than once in $bookfile\n"; 
	    }

	    $entry = 1;
	}
	else {
	    print NEW "$_";
	}
    }
   
    close(IN);
    close(NEW);
    
    system("mv $dbDir/gradesTmp.txt $bookfile");

    if ( !$entry ) {

	open(OUT, ">>$bookfile") || die "Cannot open $bookfile\n";
	$new = "$modNumber $opt_G $modDirectory $ovGrade"; 
	print OUT "$new\n";
	print "  gradeModule> Wrote \"$new\" to $bookfile\n ";
	close(OUT);

    }

    # -- finalGrade.txt
    print GRAD "FINAL DB-GRADE $opt_G\n";
    close(GRAD);


    # -- Update all final Grades in directories of the same module 
    print "\n  gradeModule> Updating the module DB ...\n";
    foreach $d (@dir )  {

	chop($d);
	if (-e "$d/finalGrade.txt" ) { system("rm -f $d/finalGrade.txt"); }
	    
	system("cp tempFinalGrade.txt $d/finalGrade.txt");

	print "  gradeModule> Wrote grade \"FINAL DB-GRADE $opt_G\" to $d/finalGrade.txt\n";
    }

    system("rm -f tempFinalGrade.txt");

    # ============== Finished updating all the files with new Grade ==================================
}

print "\n";
 
