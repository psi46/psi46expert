#! /usr/bin/perl


# Usage: phpSummaryTest.pl -m M0090/T-10a
# -----

# ----------------------------------------------------------------------

print "\nphpSummaryTestShort> Starting ...\n";

use Getopt::Std;
getopts('m:h');

$nChips = 15;

if ( $opt_h ) {
    
    $nChips = 7;
}

$dbDir    = "/home/l_tester/ptr/moduleDB";
$testDir  = "/home/l_tester/ptr/supervisor/tests";
$critFile = "/home/l_tester/ptr/moduleDB/macros/criteria-short.dat";

@summaryFiles = `ls -1 $dbDir/$opt_m/summary_*.txt`;

$testFile     = "ShortTest.root";
$phCalFile    = "phCalibration_C0.dat";
$ivFile       = "iv.dat";
if ( -e "$dbDir/$opt_m/trimParameters50_C0.dat" ) { 

    $trimFile  = "trimParameters50_C0.dat";
}
elsif ( -e "$dbDir/$opt_m/trimParameters60_C0.dat" ) {

    $trimFile  = "trimParameters60_C0.dat";
}
 


@IN = split(/\//,$opt_m);

$mainD = $IN[$#IN-1];
$subD  = $IN[$#IN];

($moduleNr, $nada) = split(/\-/, $mainD);
$testNr            = $subD;

$redTestNr = substr($testNr,2,3);
$sollTemp  = substr($testNr,1,3);

$logDir   = "$dbDir/shortTests/$mainD";

$meanT=-100.;
$sigmaT=-100.;

$meanC=-100.;
$sigmaC=-100.;
 
my $deadTot=0, $maskTot=0, $bumpsTot, $trimbitsTot, $addressTot, $gainTot=0, $pedTot=0, $par1Tot=0, 
    $badROCS=0, $badROCS1=0, $badROCS2=0, $rootTot=0;

my $badDEAD1=0, $badMASK1=0, $badBUMPS1=0, $badTRIMBITS1=0, $badADDRESS1=0, $badGAIN1=0, $badPED1=0, $badPAR1=0;
my $badDEAD2=0, $badMASK2=0, $badBUMPS2=0, $badTRIMBITS2=0, $badADDRESS2=0, $badGAIN2=0, $badPED2=0, $badPAR2=0;

my $mPar1, $sPar1, $mGain, $sGain, $mPed, $sPed;
my @Par1, @Gain, @Ped;
my $nPar1=0, $nGain=0, $nPed=0;

my $defB, $defC, $maskC, $curB, $curC, $sloB, $sloC;
my $parB, $parC, $gainB, $gainC, $pedB, $pedC;

my $current=0, $error=0;

&getGrading($critFile);
&parseLog($logDir, $moduleNr, $testNr);

push(@Par1, "<FONT COLOR=#9900ff> Par1 ");
push(@Gain,   "<FONT COLOR=#3366ff> Gain ");
push(@Ped,    "<FONT COLOR=#ff9c33> Ped ");

if (-e "$dbDir/$opt_m/summaryTest.txt" ) {system("rm -f $dbDir/$opt_m/summaryTest.txt");}
open(SUMM, ">$dbDir/$opt_m/summaryTest.txt") || die "==> Cannot open $dbDir/$opt_m/summaryTest.txt\n";

if (-e "$dbDir/$opt_m/gradingTest.txt" ) {system("rm -f $dbDir/$opt_m/gradingTest.txt");}
open(GRAD, ">$dbDir/$opt_m/gradingTest.txt") || die "==> Cannot open $dbDir/$opt_m/gradingTest.txt\n";

# -- Loop over all summary files and extract relevant numbers from 

print "\nphpSummaryTestShort> test-Nr: $redTestNr and Soll-Temp: $sollTemp\n";

$file = "$dbDir/$opt_m/../summaryTemp.txt";
if ( -e $file ) {
    getTemp($file);
}
 
foreach $file ( @summaryFiles ) {

    chop($file);
    if ( -e $file ) {
	&getDefects($file);
	&getFits($file);
    }
}

$mPar1 = $mPar1/$nChips;
$sPar1 = $sPar1/$nChips;
$mGain   = $mGain/$nChips;
$sGain   = $sGain/$nChips;
$mPed    = $mPed/$nChips;
$sPed    = $sPed/$nChips;

push(@Par1, " ? </FONT>");
push(@Gain, " ? </FONT>");
push(@Ped, " ? </FONT>");

&mkSummaryTest;
&checkCompletness("$dbDir/$opt_m", $moduleNr, $subD);

print "\nphpSummaryTestShort> $dbDir/$opt_m\/summaryTest.txt produced...\n";
print "\nphpSummaryTestShort> ................................................ finished\n\n";


# ----------------------------------------------------------------------
sub getDefects() {
    my ($f) = @_;
    
    my $roc, $dead, $mask, $bumps, $trimbits, $address, $root;
    ($bla, $roc) = split(/summary_C/, $f);
    $roc =~ s/\.txt//;
    open(IN, "$f") || die "Cannot open $file\n";
    while (<IN>) {
	if (/nDeadPixel/) {
	    s/nDeadPixel: //g;
	    $dead = $_;
	}

	if (/nDeadTrimbits/) {
	    s/nDeadTrimbits: //g;
	    $trimbits = $_;
	}

	if (/nDeadBumps/) {
	    s/nDeadBumps: //g;
	    $bumps = $_;
	}

	if (/nMaskDefect/) {
	    s/nMaskDefect: //g;
	    $mask = $_;
	}

	if (/nAddressProblems/) {
	    s/nAddressProblems: //g;
	    $address = $_;
	}

	if (/nGainDefect/) {
	    s/nGainDefect: //g;
	    $gain = $_;
	}

	if (/nPedDefect/) {
	    s/nPedDefect: //g;
	    $ped = $_;
	}

	if (/nParDefect/) {
	    s/nParDefect: //g;
	    $par1 = $_;
	}

	if (/nRootFileProblems/) {
	    s/nRootFileProblems: //g;
	    $root = $_;
	}
    }
      
    $bumps    -= $dead;
    if($bumps<0)    {$bumps    = 0;}
    $trimbits -= 4*$dead;
    if($trimbits<0) {$trimbits = 0;}
    $address  -= $dead; 
    if($address<0)    {$address    = 0;}
    $gain     -= $dead;
    if($gain<0)     {$gain     = 0;}
    $ped      -= ($dead + $gain);
    if($ped<0)      {$ped      = 0;}
    $par1     -= ($dead + $gain + $ped);
    if($par1<0)     {$par1     = 0;}

    $deadTot += $dead;
    $maskTot += $mask;
    $bumpsTot += $bumps;
    $trimbitsTot += $trimbits;
    $addressTot += $address;
    $gainTot += $gain;
    $pedTot += $ped;
    $par1Tot += $par1;
    $rootTot += $root;

    $totalDef = $dead + $bumps + $trimbits + $address + $gain + $ped + $par1;

    if($dead>$defB     && $dead<$defC    ) { $badDEAD1++;     }
    if($bumps>$defB    && $bumps<$defC   ) { $badBUMPS1++;    }
    if($trimbits>$defB && $trimbits<$defC) { $badTRIMBITS1++; }
    if($address>$defB  && $address<$defC ) { $badADDRESS1++;  }
    if($gain>$defB     && $gain<$defC    ) { $badGAIN1++;     }
    if($ped>$defB      && $ped<$defC     ) { $badPED1++;      }
    if($par1>$defB     && $par1<$defC    ) { $badPAR1++;      }

    if($dead>$defC    ) { $badDEAD2++;     }
    if($mask>=$maskC   ) { $badMASK2++;     }
    if($bumps>$defC   ) { $badBUMPS2++;    }
    if($trimbits>$defC) { $badTRIMBITS2++; }
    if($address>$defC ) { $badADDRESS2++;  }
    if($gain>$defC    ) { $badGAIN2++;     }
    if($ped>$defC     ) { $badPED2++;      }
    if($gar1>$defC    ) { $badPAR2++;      }

    if ($totalDef > $defB )  { $badROCS++; }
    if ($totalDef > $defB && 
	$totalDef < $defC)   { $badROCS1++; }
    if ($totalDef > $defC )  { $badROCS2++; }
 
#    if($dead>$defB || $mask>$defB || $bumps>$defB || $trimbits>$defB || $address>$defB)   { $badROCS++;} 
#    if(($dead>$defB || $mask>$defB || $bumps>$defB || $trimbits>$defB || $address>$defB) &&
#       ($dead<$defC || $mask<$defC || $bumps<$defC || $trimbits<$defC || $address<$defC)) { $badROCS1++;} 
#    if($dead>$defC || $mask>$defC || $bumps>$defC || $trimbits>$defC || $address>$defC)   { $badROCS2++;} 

}

#----------------------------------------------------------------------------------------

sub getTemp() {
    my ($f) = @_;

    open(TE, "$f") || die "Cannot open $f\n"; 

    while (<TE>) {
	if (/$redTestNr/) {
	    ($nr, $meanT, $sigmaT) = split(/ /, $_);
	    chop($sigmaT);
	    print "phpSummaryTestShort> TEMP $meanT $sigmaT\n";
	}

	if (/T-cycl/) {
	    s/T-cycl //g;
	    ($meanC, $sigmaC) = split(/ /, $_);
	    chop($sigmaC);
	    print "phpSummaryTestShort> CYCL $meanC $sigmaC\n";
	}

    }
}

#----------------------------------------------------------------------------------------

sub getFits() {
    my ($f) = @_;
    
    my $nSCurveChip, $mSCurveChip, $sSCurveChip, $nThrChip, $mThrChip, $sThrChip;
    my $nPar1Chip, $mPar1Chip, $sPar1Chip;
    my $nGainChip, $mGainChip, $sGainChip, $nPedChip, $mPedChip, $sPedChip;

    ($bla, $roc) = split(/summary_/, $f);
    $roc =~ s/\.txt//;
    open(IN, "$f") || die "Cannot open $f\n";    

    while (<IN>) {
	if (/SCurve/) {
	    s/SCurve //g;
	    ($nSCurveChip, $mSCurveChip, $sSCurveChip) = split(/ /);
	    chop($sSCurveChip);
	}

	if (/Threshold/) {
	    s/Threshold //g;
	    ($nThrChip, $mThrChip, $sThrChip) = split(/ /);
	    chop($sThrChip);
	}

	if (/Gain/) {
	    s/Gain //g;
	    ($nGainChip, $mGainChip, $sGainChip) = split(/ /);
	    chop($sGainChip);
	}

	if (/Pedestal/) {
	    s/Pedestal //g;
	    ($nPedChip, $mPedChip, $sPedChip) = split(/ /);
	    chop($sPedChip);
	}

	if (/Parameter1/) {
	    s/Parameter1 //g;
	    ($nPar1Chip, $mPar1Chip, $sPar1Chip) = split(/ /);
	    chop($sPar1Chip);
	}

    } 
    
    # -------> Qualification parameters (only for comments, not grading yet!)
    
    if ( ($mPar1Chip > $parB) || ($nPar1Chip < ($defB - 42)) )     {

	push(@Par1, $roc); $nPar1++; 
    }
    	 
    if ($mGainChip != 0) {
	
	if ( ($sGainChip/$mGainChip > $gainB)  || ($nGainChip < ($defB - 42)) ) { 
	    
	    push(@Gain, $roc); $nGain++; 
	}
	
	if ( (65*$sPedChip > $pedB)  || ($nPedChip < ($defB - 42)) ) { 
	    $tmp = 65*$sPedChip;
	    push(@Ped, $roc);  $nPed++; 
	}
    }   
    
    $mSCurve += $mSCurveChip;
    $sSCurve += $sSCurveChip;
    $mThr    += $mThrChip;
    $sThr    += $sThrChip;
    $mGain   += $mGainChip;
    $sGain   += $sGainChip;
    $mPed    += $mPedChip;
    $sPed    += $sPedChip;
    $mPar1   += $mPar1Chip;
    $sPar1   += $sPar1Chip;
}

#----------------------------------------------------------------------------------------
sub mkSummaryTest() {

  print GRAD "DEAD $badDEAD1 $badDEAD2\n";
  print GRAD "MASK $badMASK1 $badMASK2\n";
  print GRAD "BUMPS $badBUMPS1 $badBUMPS2\n";
  print GRAD "TRIMBITS $badTRIMBITS1 $badTRIMBITS2\n";
  print GRAD "ADDRESS $badADDRESS1 $badADDRESS2\n";

  print GRAD "GAIN_BC $badGAIN1 $badGAIN2\n";
  print GRAD "PEDESTAL_BC $badPED1 $badPED2\n";
  print GRAD "PARAMETER1_BC $badPAR1 $badPAR2\n";
 
  print SUMM "Directory $opt_m \n";
  print SUMM "ModuleNr TestNr $moduleNr $testNr\n";
  print SUMM "Defects $deadTot $maskTot $par1Tot $gainTot $pedTot \n";
  print SUMM "ROCS with defects > 1% $badROCS $badROCS1 $badROCS2\n";
  print SUMM "Missing histograms $rootTot\n";
  
  if ( -e "$testDir/$opt_m/$testFile") {
      $date=getFileDate($testDir, $testFile); print SUMM "Tested yes on $date\n";
  }
  elsif ( -e "$dbDir/$opt_m/$testFile") {  
      $date=getFileDate($dbDir, $testFile); print SUMM "Tested yes on $date\n";
  }      
  else {print SUMM "Tested no\n";}
  

  if ( -e "$testDir/$opt_m/$trimFile") {
      $date=getFileDate($testDir, $trimFile); print SUMM "Trimming yes on $date\n";
  }
  elsif ( -e "$dbDir/$opt_m/$trimFile") {  
      $date=getFileDate($dbDir, $trimFile); print SUMM "Trimming yes on $date\n";
  }  
  else {print SUMM "Trimming no\n";}


  if ( -e "$testDir/$opt_m/$phCalFile") {
      $date=getFileDate($testDir, $phCalFile); print SUMM "phCalibration yes on $date\n";
  }    
  elsif ( -e "$dbDir/$opt_m/$phCalFile") {  
      $date=getFileDate($dbDir, $phCalFile); print SUMM "phCalibration yes on $date\n";
  }    
  else {print SUMM "phCalibration no\n";}


  if ( $meanT != -100 && $sigmaT != -100 ) {
      print SUMM "Temp $meanT $sigmaT Soll $sollTemp\n";
  }
  else {
      print SUMM "Temp $sollTemp $sigmaT Soll $sollTemp\n";
  }

  if (`ls -1 $dbDir/$opt_m/../T-10a` && `ls -1 $dbDir/$opt_m/../T-10b`)  {
      
      print SUMM "Thermal cycling yes $meanC $sigmaC\n";
  }               
  else {print SUMM "Thermal cycling no $meanC $sigmaC\n";}
  
  print SUMM "I 150 $current\n";
  print SUMM "LogErrors $error\n";

  print SUMM "SCurve $mSCurve $sSCurve \n";
  print SUMM "Threshold   $mThr $sThr \n";
  print SUMM "Par1 $mPar1 $sPar1 \n";
  print SUMM "Gain $mGain $sGain \n";
  print SUMM "Pedestal $mPed $sPed \n";

#comments
  if ($nPar1 > 0 ) { print SUMM "@Par1\n"; }
  else { print SUMM "Par1 good\n"; }
  if ($nGain > 0 )  { print SUMM "@Gain\n"; }
  else { print SUMM "Gains good\n"; }
  if ($nPed > 0 )   { print SUMM "@Ped\n"; }
  else { print SUMM "Peds good\n"; }

}

# ----------------------------------------------------------------------
sub getFileDate() {
    local($fileDir, $file) = @_;
    
 ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks)
       = stat("$fileDir/$opt_m/$file");
       
 $time = localtime($mtime);
  
 return $time;
}

#---------------------------------------------------------------------------------------------------------------------
sub checkCompletness() {
    my ($dir, $mod, $temp) = @_;

    print "\nphpSummaryTestShort> Checking completeness of directory $dir ($nChips chips)\n";

    open(MISS, ">$dir/missing.txt") || die "==> Cannot open $dir/missing.txt";
    print MISS "Missing: ";

    $ok = 1;
    $ic = 1;

    # directories
    if( !((-d "$dir/../T-10a") && (-d "$dir/../T+17a")) ) { 

	$ok = 0; 
	$ic = 0; 
	print MISS "Directory - Test Incomplete! ";
    }

    #summaryTest.txt
    ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks)
              = stat("$dir/summaryTest.txt");
 
    if( ! $size ) { 

	$ok = 0; 
	print MISS "summaryTest.txt ";

    }

 
    #ShortTest.root
    if( !(-e "$dir/ShortTest.root") ) {

	$ok = 0; 
	print MISS "ShortTest.root";
    }

    #Relevant Histograms in ShortTest.root
    open(IN, "$dir/summaryTest.txt") || die "Cannot open $dir/summaryTest.txt\n";

    while (<IN>) {
	if (/histograms/) {
	    s/Missing histograms //g;
	    $histo = $_;
	}
    }

    if ( $histo > 0 ) {

	$ok = 0;
	chop($histo);
	print MISS "$histo histograms. "; 
    }

    print MISS "Files: ";
 
    #moduleSummary - can not be checked at that moment !
#    if( !(-e "$dir/$mod$temp.gif") ) { 
#
#	$ok = 0; 
#	print MISS "$mod$temp.gif ";
#    }

    #phCalibration_Cxx.dat
    $#phcal = -1;
    if ( (-e  "$dir/phCalibration_C0.dat") || (-e  "$dir/phCalibration_C8.dat") ) {
	@phcal = `ls -1 $dir/phCalibration_C*.dat`;
    }

    if( $#phcal < $nChips ) { 
	
	$ok = 0;
	print MISS "phCalibration ";
    }

    #phCalibrationFit_Cxx.dat
    $#phcalF = -1;
    if ( (-e  "$dir/phCalibrationFit_C0.dat") || (-e  "$dir/phCalibrationFit_C8.dat") ) {
	@phcalF = `ls -1 $dir/phCalibrationFit_C*.dat`;
    }

    if( $#phcalF < $nChips ) { 
	
	$ok = 0;
	print MISS "phCalibrationFit ";
    }

    #summary_Cxx.dat
    $#sums = -1;
    if ( (-e  "$dir/summary_C0.txt") || (-e  "$dir/summary_C8.txt") ) {
	@sums = `ls -1 $dir/summary_C*.txt`;
    }

    if( $#sums < $nChips ) { 

	$ok = 0;
	print MISS "summary-text "; 
    }

    #Cxx.gif
    $#gifC = -1; 
    if ( (-e  "$dir/C0.gif") || (-e  "$dir/C8.gif") ) {
	@gifC = `ls -1 $dir/C*.gif`;
    }

    if( $#gifC < $nChips ) { 

	$ok = 0;
	print MISS "chip-gif "; 
    }

    print MISS "\n"; 

    #-------------------------------------------------------------------------------------------------------------------

    close(MISS);
		
    if ( "-e $dir/comment_3.txt" ) { 

	system("rm -f $dir/comment_3.txt"); 
    }  

    if ( !$ok ) {

	system("/bin/cat $dir/missing.txt > $dir/comment_3.txt");
	if ( $ic ) { print "\nphpSummaryTestShort> !!!!!!!!!  There are files missing in directory $dir !!! \n\n"; }
    }
    else {

        print "phpSummaryTestShort> ... complete ! \n";
    }

    system("rm -f $dir/missing.txt");

}

# ----------------------------------------------------------------------
sub getGrading() {
    my ($f) = @_;
    
    open(IN, "$f") || die "Cannot open $file\n";
    while (<IN>) {

	if (/defectsB/) {
	    s/defectsB //g;
	    $defB = $_;
	    chop($defB);
	}

	if (/defectsC/) {
	    s/defectsC //g;
	    $defC = $_;
	    chop($defC);
	}

	if (/maskdefC/) {
	    s/maskdefC //g;
	    $maskC = $_;
	    chop($maskC);
	}

	if (/currentB/) {
	    s/currentB //g;
	    $curB = $_;
	    chop($curB);
	}

	if (/currentC/) {
	    s/currentC //g;
	    $curC = $_;
	    chop($curC);
	}

	if (/slopeivB/) {
	    s/slopeivB //g;
	    $sloB = $_;
	    chop($sloB);
	}

	if (/slopeivC/) {
	    s/slopeivC //g;
	    $sloC = $_;
	    chop($sloC);
	}


	if (/noiseB/) {
	    s/noiseB //g;
	    $noiB = $_;
	    chop($noiB);
	}

	if (/noiseC/) {
	    s/noiseC //g;
	    $noiC = $_;
	    chop($noiC);
	}

	if (/trimmingB/) {
	    s/trimmingB //g;
	    $trmB = $_;
	    chop($trmB);
	}

	if (/trimmingC/) {
	    s/trimmingC //g;
	    $trmC = $_;
	    chop($trmC);
	}

	if (/gainB/) {
	    s/gainB //g;
	    $gainB = $_;
	    chop($gainB);
	}

	if (/gainC/) {
	    s/gainC //g;
	    $gainC = $_;
	    chop($gainC);
	}

	if (/pedestalB/) {
	    s/pedestalB //g;
	    $pedB = $_;
	    chop($pedB);
	}

	if (/pedestalC/) {
	    s/pedestalC //g;
	    $pedC = $_;
	    chop($pedC);
	}

	if (/par1B/) {
	    s/par1B //g;
	    $parB = $_;
	    chop($parB);
	}

	if (/par1C/) {
	    s/par1C //g;
	    $parC = $_;
	    chop($parC);
	}
    }
}


# ----------------------------------------------------------------------
sub parseLog() {
    my ($dir, $m, $t) = @_;

    open(IN, "$dir/sv.log") || die "cannot open $dir/sv.log\n";

    my $mod = -1; 
    if ( $t eq "T-10a" ) { $temp = "-10"; }
    if ( $t eq "T+17a" ) { $temp = "17"; }

    while (<IN>) {
	
	if (/Setup jumo at T = $temp/) {

	    while (<IN>) {
		next unless (/currentTest for module [0-9]: (M[0-9]{4})/);
		$mod = $1; 
		# print "-> $mod\n";
		next unless ($mod eq $m); 
		while (<IN>) {
		    next unless (/Stable within 0.05: .* -> (.*)/);
		    $current = $1; 
		    # print "  T=$temp $mod $current10\n";
		    $current *= 1e6;
		    goto done; 
		}
	    }
	}
	
    }
    
    close(IN); 
    
  done: 
    print "File: $dir/sv.log\n";
    printf("module %s: current at $temp %3.3f uA\n", $mod, $current);
    
    
    open(IN, "$dir/T-10a/46.out") || die "cannot open $dir/T-10a/46.out\n";
    while (<IN>) {
	if (/>>>>/) {
	    $error++; 
	    print "---> ERROR in file $dir/T-10a/46.out\n";
	}
    }
    close(IN); 
    
    open(IN, "$dir/T+17a/46.out") || die "cannot open $dir/T+17a/46.out\n";
    while (<IN>) {
	if (/>>>>/) {
	    $error++; 
	    print "---> ERROR in file $dir/T+17a/46.out\n";
	}
    }
    close(IN); 
    
    if ($error < 1) {
	print "              no error found. \n";
    } else {
	print "\n\n !!!!!!!!&%&%&%&%&%&%&% check log-file errors  &%&%&%&%&%&%&%!!!!!!!!!!! \n\n";
    }
}
