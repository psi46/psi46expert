#!/usr/bin/perl
#
#
#  USAGE cd moduleDB; ./web_upd.pl -<options> -<dir>
#
#                  =================================
#  FULL ANALYSIS    ./web_upd.pl -A M0090/T-10a    
#                  =================================
#
#------------------------------------------------------
#                    OPTIONS                          #
# -----------------------------------------------------
# -p 0,1,2,3 phCalibration
# -s SCurve
# -c chipSummaryPage
# -m moduleSummaryPage

# -C " ... " add your comment
# -G " ... " add the final grade
# -F " ... " add mandatory comment 
#            when regrading different to overall grade

# -d set(create new) target directory for data on kamor
# -w produces html-files & transfers plots etc.
#    to kamor
# -r option to be added for retested modules
# -h option to be added for half modules
#
# -A full analysis plus data transfer to kamor
# -H full analysis of half modules plus data transfer to kamor
# -R full analysis of retested modules (no update to kamor)
# -S short analysis plus data transfer to kamor
# -----------------------------------------------------
  

use Getopt::Std;
getopts('C:G:F:N:i:d:H:h:ARrZSzP:pscmwu');

if ($opt_S) { $opt_Z = 1; }  # temppppppppppppppppppppppppppppppppp

if ($opt_A) {  $opt_p=1; $opt_P=3; $opt_s=1; $opt_c=1; $opt_m=1; $opt_w=1; } 
if ($opt_R) {  $opt_p=1; $opt_P=3; $opt_s=1; $opt_c=1; $opt_m=1; $opt_w=1; $opt_r=1; } 
if ($opt_H) {  $opt_p=1; $opt_P=3; $opt_s=1; $opt_c=1; $opt_m=1; $opt_w=1; $opt_h=$opt_H; }  
if ($opt_Z) {  $opt_p=1; $opt_P=3; $opt_s=0; $opt_c=1; $opt_m=1; $opt_w=1; $opt_z=1; }  
#if ($opt_Z) {  $opt_p=1; $opt_P=3; $opt_s=0; $opt_c=1; $opt_m=1; $opt_w=1; $opt_h=$opt_H; } 
#if ($opt_Z) {  $opt_p=0; $opt_P=0; $opt_s=0; $opt_c=1; $opt_m=1; $opt_w=1; $opt_h=$opt_H; }

# temporary update settings
if ($opt_u ) { $opt_p=0; $opt_P=0; $opt_s=0; }

# current information for modules that could NOT be tested (only together with $opt_N)
if ( !$opt_i ) {

    $opt_i = 0;
}
# half-modules
if ( !$opt_h ) {

    $opt_h="m";
}

# short-tests
if ( $opt_z ) {

    $opt_d = "shortTests";
}

# ph-Fit settings
if ( $opt_p ) {

    $opt_P = 3;
}

if ( $opt_P && !$opt_p ) {

    $opt_p = 1;
}

if ( $opt_P && ($opt_P != 0 && $opt_P != 1 && $opt_P != 2 && $opt_P != 3) ) {

    print "\n\nWEB UPDATE> PHCalibration-Fit macro does not run with option \"-P $opt_p!\"!!\n";
    print "WEB UPDATE> Please chose option 0,1,2 or 3.... \n\n";
    print "WEB UPDATE> ******  ABORTING web update !!!\n\n";
    exit;
}

# comments
if ( $opt_C && $opt_N ) {

    print "\n\nWEB UPDATE> The options -N and -C cannot be used together!\"!!\n";
    print "WEB UPDATE> ******  ABORTING web update !!!\n\n";
    exit;
}

# directories
if ( $opt_d ) {

    print "\n\nWEB UPDATE> ******  Your output will be saved in a seperate directory ($opt_d) on kamor!!!\n\n";
}

$numArgs = $#ARGV + 1;
print "WEB UPDATE> $numArgs module test(s) will be processed.\n";

$kamor = "l\_tester\@kamor.ethz.ch";
$dbDir = "/home/l_tester/ptr/moduleDB";
$rtDir = "/home/l_tester/ptr/retestedModules";


if ($opt_z) {

    $rootFile = "ShortTest.root";

} else {
    
    $rootFile = "FullTest.root";
}

# ================================== BOOK-KEEPING =============================================
print "\nWEB UPDATE> Backing-up book-keeping ... \n";

chdir("$dbDir/book-keeping");
@tobk = `ls -1 *.dat `;
@bk   = `ls -1 backup/*.dat `; # for later: sort back-ups into directories or delete them if > 50

($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
$year = sprintf("%02d", $year % 100);

$sname =  sprintf("%02d%02d%02d.%02d:%02d", $year, $mon+1, $mday, $hour, $min); # mon is off by 1??
$smonth = sprintf("%02d", $mon+1);
$syear  = sprintf("%02d", $year);

# organize
if ( -e "backup/$smonth" ) {
    system("/bin/mv backup/$syear$smonth*.dat backup/$smonth/");
}
else {

    $lmonth = sprintf("%02d", $mon);
    system("/bin/mv backup/$syear$lmonth*.dat backup/$lmonth/");

    system("mkdir backup/$smonth");
    system("/bin/mv backup/$syear$smonth*.dat backup/$smonth/");
}

# make back-up copy
foreach $file1 ( @tobk ) {

    chop($file1);
    $file1 =~ s/\.dat//;


    system("/bin/cp $file1.dat backup/$sname-$file1.dat");
}


chdir("$dbDir");

# ================================== PART 1 ==================================================

foreach $argnum (0 .. $#ARGV) {

    $dirTmp   = $ARGV[$argnum];    
    $dirMacros = "../$dirTmp";
    $dirDB = $dirTmp;
    
    @IN = split(/\//,$dirDB);
    
    $mainD = $IN[$#IN-1];
    $subD  = $IN[$#IN];
    
    ($moduleNr, $nada) = split(/\-/, $mainD);
    $testNr            = $subD;

    $dirKamor = $moduleNr."/".$testNr;

    if ( $opt_d ) {
	$logDir   = "$dbDir/$opt_d/$mainD";
    } else {
	$logDir   = "$dbDir/$mainD";
    }

    # Comments  
    if ($opt_C) {
	
	if (-e "$dirDB/comment_1.txt") { system("rm -f $dirDB/comment_1.txt"); }

	open(COMM, ">$dirDB/comment_1.txt") || die "==> Cannot open $dirDB/comment_1.txt\n";
	print COMM "Comment: $opt_C \n";
	
	print "\nWEB UPDATE> Wrote comment \"$opt_C\" to file:\n";
	print "WEB UPDATE> $dirDB/comment_1.txt ...\n\n";
    }

    #Grades
    if ($opt_G) {
	
	checkGrades("$dirDB/summaryTest.txt");

	# Regrading comment mandatory in case final grade != overall grade  

	if ($opt_F) {
	    
	    if (-e "$dirDB/comment_2.txt") { system("rm -f $dirDB/comment_2.txt"); }
	    
	    open(COMM, ">$dirDB/comment_2.txt") || die "==> Cannot open $dirDB/comment_2.txt\n";
	    print COMM "Regrading: $opt_F \n";
	    
	    print "\nWEB UPDATE> Wrote regrading comment \"$opt_F\" to file:\n";
	    print "WEB UPDATE> $dirDB/comment_2.txt ...\n\n";
	}

	if ( !$opt_F && ($ovGrade ne $opt_G) ) {

	    print  "WEB UPDATE> ***** Grading inconsistency !!!  *****\n";
	    print  "WEB UPDATE> Overall Grade $ovGrade -->  your grade $opt_G \n";
	    print  "WEB UPDATE> Please comment your choice of grade using:\n\n";
	    print  "              web_upd.pl -G grade -F \"comment\"\n\n";
	    print  "WEB UPDATE> ***** Aborting web_upd.pl of $dirDB ... *****\n\n";
	    exit;
	}

	if ( $opt_z ) {

	    print COMM "Regrade $opt_G \n";

	} else {

	    system("./macros/gradeModule.pl -m $dirDB -G $opt_G -F \"$opt_F\"");
	}
    }


    # Modules that can't be tested
    if ( $opt_N ) {
	
	if ( !$opt_i ) {
	    $opt_i = &findCurrent($logDir, $moduleNr);
	}
	
	&mkSummaryTest($dirDB, $opt_i, $opt_d);

	if ( $opt_z ) {

	    system("perl -pi -e \"s/OVERALL GRADE/shortTest GRADE/g\" $dirDB/summaryTest.txt");

	    &getGrade($dirDB);
	    $gline = `grep FINAL $dirDB/../finalGrade.txt`;
	    chop($gline);
	    system("perl -pi -e \"s/FINAL DB-GRADE C/$gline/g\" $dirDB/summaryTest.txt");

	} else {

	    if (-e "$dirDB/../finalGrade.txt" ) { system("rm -f $dirDB/../finalGrade.txt"); }
	    open(GRADE, ">$dirDB/../finalGrade.txt") || die "==> Cannot open $dirDB/../finalGrade.txt\n";
	    print GRADE "FINAL DB-GRADE C\n";
	    close(GRADE);
	}


	if ( -e "$dirDB/comment_4.txt")    { system("rm -f $dirDB/comment_4.txt"); }

	open(COMM, ">$dirDB/comment_4.txt") || die "==> Cannot open $dirDB/comment_4.txt\n";
	print COMM "Comment: $opt_N \n";

	print "\nWEB UPDATE> Wrote comment \"$opt_N\" to file:\n";
	print "WEB UPDATE> $dirDB/comment_4.txt ...\n\n";

	system("/bin/cat $dirDB/comment_4.txt >> $dirDB/summaryTest.txt");
	
	if ( $opt_h eq "a" || $opt_h eq "b" ) {
	    system("/bin/echo \"Half-Module $opt_h\" >> $dirDB/summaryTest.txt");
	}

    }


    # Start Part 1
    if(`ls -1 $dirDB/$rootFile`) {
	
	chdir("macros");  
	
	if ( $opt_p || $opt_s || $opt_c ) {
	    
	    open(ROOT, "|root -l") || die "Cannot open ROOT\n";  
	    
	    print ROOT ".L PHCalibration.C\n";
	    if ($opt_p)  { 

		print ROOT "FitAllCurves(\"$dirMacros\", 0)\n";

		if ( $opt_P ) {

		    print ROOT "FitAllCurves(\"$dirMacros\", $opt_P)\n"; 
		}
	    }
	    
	    print ROOT ".L SCurve.C\n";
	    if ($opt_s)  { print ROOT "FitSCurves(\"$dirMacros\")\n"; }
	    
	    if ($opt_z) {print ROOT ".L chipSummaryPageShort.C\n";}
            else {print ROOT ".L chipSummaryPage.C\n";}
	    if ($opt_c)  { print ROOT "chipSummaries(\"$dirMacros\", \"$opt_h\")\n"; }
	    
	    print ROOT ".q\n";
	    close(ROOT);
	    
	}
	
	
	
	if ($opt_m) {
	    
	    open(ROOT, "|root -l") || die "Cannot open ROOT\n";  
	    print ROOT ".L tempProfile.C\n"; 
	    print ROOT "readProfile(\"$dirMacros\")\n";
	    print ROOT ".q\n";
	    close(ROOT);
	    
            $phpSummary = "phpSummaryTest.pl";
            if ($opt_z) {$phpSummary = "phpSummaryShortTest.pl";}
            
	    if ( $opt_h eq "a" || $opt_h eq "b" ) {
		system("./$phpSummary -h -m $dirDB");    #produces text file with summary for php & mod.
	    }
	    else {
		
		system("./$phpSummary -m $dirDB");    #produces text file with summary for php & mod.
	    }

	    open(ROOT, "|root -l") || die "Cannot open ROOT\n";  
	    if ($opt_z) {print ROOT ".L moduleSummaryPageShort.C\n";}
            else {print ROOT ".L moduleSummaryPage.C\n";}
	    print ROOT "moduleSummary(\"$dirMacros\", \"$opt_h\")\n";
	    print ROOT ".q\n";
	    close(ROOT);
	}
	
	chdir("..");
    }
    
    else {
	
	if(-d $dirDB)  {  print "\nWEB UPDATE> ----> COULD NOT FIND $rootFile IN DIRECTORY $dirDB ....\n\n";  next; }
	else {   print "\nWEB UPDATE> ----> $dirDB: NO SUCH DIRECTORY .... \n\n"; next;  }
	
    }
    
}


# ================================== PART 2 ==================================================
$moduleNr = '';

foreach $argnum (0 .. $#ARGV) {

    $tmpMod = $moduleNr;

    $dirDB   = $ARGV[$argnum];    
    $dirMacros = "../$dirDB";
    $dirDB = $dirDB;
    
    @IN = split(/\//,$dirDB);
    
    $mainD = $IN[$#IN-1];
    $subD  = $IN[$#IN];
    
    ($moduleNr, $nada) = split(/\-/, $mainD);
    $testNr            = $subD;
    
    $dirKamor = $moduleNr."/".$testNr;
    
    if(`ls -1 $dirDB/$rootFile`) {
	
	chdir("macros");  
	
	if ($opt_m) {

	    if ( $opt_z ) {

		#appends overall grade and DB grade to summary file 
		if ( $opt_h ) {
		    
		    system("./phpSummaryGrading.pl -hz -m $dirDB"); 
		    
		} else {
		    
		    system("./phpSummaryGrading.pl -z -m $dirDB"); 
		}
		
	    } elsif ( $opt_r || $opt_d ) {
		
		#appends overall grade and DB grade to summary file 
		system("./phpSummaryGrading.pl -r -m $dirDB"); 
		
	    } elsif ( $opt_h eq "a" || $opt_h eq "b" ) {
		
		#appends overall and IDENT. final grade to summary file 
		system("./phpSummaryGrading.pl -h -m $dirDB");
		
	    } else {
		
		#appends overall and IDENT. final grade to summary file 
		system("./phpSummaryGrading.pl -m $dirDB");   
	    }
	    
            # -- Appending comments to summary file
            #Half-module Comment
	    if ( $opt_h eq "a" || $opt_h eq "b" ) {
		system("/bin/echo \"Half-Module $opt_h\" >> $dirMacros/summaryTest.txt");
	    }

            # General Comments
	    if ( (-e "$dirMacros/comment_1.txt") && (-e "$dirMacros/summaryTest.txt"))  {
		system("/bin/cat $dirMacros/comment_1.txt >> $dirMacros/summaryTest.txt");
	    }
	   
            # Manual Grading Comments 
	    if ( (-e "$dirMacros/comment_2.txt") && (-e "$dirMacros/summaryTest.txt"))  {
		system("/bin/cat $dirMacros/comment_2.txt >> $dirMacros/summaryTest.txt");
	    }

	    # Missing files comments
	    if ( (-e "$dirMacros/comment_3.txt") && (-e "$dirMacros/comment_3.txt"))  {
		system("/bin/cat $dirMacros/comment_3.txt >> $dirMacros/summaryTest.txt");
	    }


            # produces html-page for one module 
	    
	    if ($opt_z) { system("./htmlSummaryPageShort.pl -m $dirMacros"); }
	    else { system("./htmlSummaryPage.pl -m $dirMacros"); }   

	}

	if ($opt_w) { 


	    ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks)
		= stat("$dirMacros/summaryTest.txt");
	    
	    if ( $size!=0 ) {
		
		@path = split(/\//, $dirKamor);
	
		# update the entry in book-keeping and set(create) target directory on kamor
		if ( $opt_r ) {
		    
		    system("./bookKeeping.pl -r -m $dirDB");   
		    $fullPath = "/export/data1/moduleTests/retestedModules" ;
		    
		    # copy directory to retestedModules directory
		    if ( !(-d "$rtDir/$mainD") ){
			
			print "\nWEB UPDATE> Copying $mainD to $rtDir ... \n\n";
			system("cp -r $dbDir/$mainD $rtDir/$mainD");
		    }
		    else {
			
			print "\nWEB UPDATE> Directory $mainD was already copied to $rtDir ... \n\n";
		    }
		}
		elsif ( $opt_d ) {
		    
		    system("./bookKeeping.pl -d $opt_d -m $dirDB");   
		    $fullPath = "/export/data1/moduleTests/moduleDB/$opt_d" ;
		    system("ssh $kamor mkdir $fullPath"); 
		}
		elsif ( $opt_h eq "a" || $opt_h eq "b" ) {
		    
		    system("./bookKeeping.pl -h -m $dirDB");   
		    $fullPath = "/export/data1/moduleTests/moduleDB" ; 
		}
		else {  
		
		    system("./bookKeeping.pl -m $dirDB");             
		    $fullPath = "/export/data1/moduleTests/moduleDB" ; 
		}

                
		# move an old test to directory "old" on kamor first
		if ( ($tmpMod ne $moduleNr) && !$opt_C && !$opt_G ) {

		    print "WEB UPDATE> Removing directory $fullPath/old/$moduleNr on kamor ....\n";
		    system("ssh $kamor rm -rf $fullPath/old/$moduleNr");
		    print "WEB UPDATE> Moving directory $fullPath/$moduleNr to $fullPath/old/ on kamor ....\n";
		    system("ssh $kamor mv $fullPath/$moduleNr $fullPath/old/"); 

		} 

		# create target sub-directories on kamor
		foreach $p (0 .. $#path) {

		    $fullPath = "$fullPath/$path[$p]";

		    print "WEB UPDATE> Creating directory $fullPath on kamor ....\n";
		    system("ssh $kamor mkdir $fullPath"); 

		}		

		# starting data transfer
		print "WEB UPDATE> Copying everything to $fullPath on kamor ....\n";
		system("scp $dirMacros/*.html $kamor:$fullPath");
		system("scp $dirMacros/*.gif $kamor:$fullPath");
		system("scp $dirMacros/summaryTest.txt $kamor:$fullPath");
		system("scp $dirMacros/../*.gif $kamor:$fullPath/../");
	    }
	    
	    
	}
	
	chdir("..");
    }
    
    else {
	
	if(-d $dirDB)  {  print "\nWEB UPDATE> ----> COULD NOT FIND $rootFile IN DIRECTORY $dirDB ....\n\n";  next; }
	else {   print "\nWEB UPDATE> ----> $dirDB: NO SUCH DIRECTORY .... \n\n"; next;  }
	
    }
    
}


# ----------------------------------------------------------------------
sub checkGrades() {
    my ($f) = @_;

    $ovGrade = "";

    if (-e "$f" ) {
	
	open(GR, "$f") || die "Cannot open $f\n"; 
	
	while (<GR>) {
	    
	    if (/OVERALL /) {
		
		s/OVERALL GRADE //g;
		$ovGrade = $_;
		chop($ovGrade);
	    }
	}
	
	close(GR);
    }

    if ( $ovGrade eq "" ) {

	$ovGrade = $opt_G;
    }
}
# ----------------------------------------------------------------------
sub mkSummaryTest() {
    my ($dir, $current, $dir2) = @_;

    print "\n\t %%%%%%%% creating summaryTest.txt file %%%%%%%% \n\n";

    @IN = split(/\//,$dir);

    $mainD = $IN[$#IN-1];
    $subD  = $IN[$#IN];

    if ( !(-d "$dir"))        { system("mkdir -p $dir"); }
    
    ($moduleNr, $nada) = split(/\-/, $mainD);
    $testNr            = $subD;
    $sollTemp  = substr($testNr,1,3);

    if ( -e "$dir/summaryTest.txt" ) { system("cp $dir/summaryTest.txt $dir/summaryTest.txt.OLD"); }

    system("cp summaryTest.txt $dir/summaryTest.txt\n"); 

    if ( $dir2 ) {           
	system("perl -pi -e \"s/moduleDirectory/$dir2\\\/$mainD\\\/$subD/g\" $dir/summaryTest.txt");
    } else {
	system("perl -pi -e \"s/moduleDirectory/$mainD\\\/$subD/g\" $dir/summaryTest.txt");
    }

    system("perl -pi -e \"s/moduleNumber/$moduleNr/g\" $dir/summaryTest.txt");
    system("perl -pi -e \"s/testNumber/$testNr/g\" $dir/summaryTest.txt");
    system("perl -pi -e \"s/temperature/$sollTemp/g\" $dir/summaryTest.txt");

    if( $current > 0 ) {
	system("perl -pi -e \"s/current/$current/g\" $dir/summaryTest.txt");
    } else {
	system("perl -pi -e \"s/current/-/g\" $dir/summaryTest.txt");
    }

    if ( !(-e "$dir/$rootFile") ) { system("touch $dir/$rootFile"); }
}



# ----------------------------------------------------------------------
sub getGrade() {
    my ($dir) = @_;

    print "\n\t %%%%%%%% scanning DB grades for $moduleNr %%%%%%%% \n\n";

    chdir("$dbDir/macros");

    system("./scanDBgrades.pl -m $dir");
    
    chdir("$dbDir");

}

# ----------------------------------------------------------------------
sub findCurrent() {
    my ($dir, $m) = @_;
    
    $current = 0;

    if ( !(-e "$dir/sv.log") ) {

	return $current;
    }

    open(IN, "$dir/sv.log") || die "cannot open $dir/sv.log\n";

    my $mod = -1; 
    my $t10 = 0; 

    while (<IN>) {
	
	if (/Setup jumo at T = -10/) {

	    while (<IN>) {
		next unless (/currentTest for module [0-9]: (M[0-9]{4})/);
		$mod = $1; 
		# print "-> $mod\n";
		next unless ($mod eq $m); 
		while (<IN>) {
		    next unless (/WARNING: LARGE currents drawn: (.*)  disabling testboard/);
		    $current = $1; 
		    # print "  T=-10 $mod $current\n";
		    $current *= 1e6;
		    $t10 = 1;
		    goto done; 
		}
	    }
	}
	
    }
    
    close(IN); 
    
  done: 

    if ( !$t10 ) {  $current = 0; }
    print  "WEB UPDATE> File: $dir/sv.log\n";
    printf("WEB UPDATE> module %s: current at $temp %3.3f uA\n", $mod, $current);

    return $current;
  
}
