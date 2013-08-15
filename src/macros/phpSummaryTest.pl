#! /usr/bin/perl


# Usage: phpSummaryTest.pl -m M0090/T-10a
# -----

# ----------------------------------------------------------------------

print "\nphpSummaryTest> Starting ...\n";

use Getopt::Std;
getopts('m:h');

$nChips = 15;

if ( $opt_h ) {
    
    $nChips = 7;
}

$dbDir    = "/home/l_tester/ptr/moduleDB";
$testDir  = "/home/l_tester/ptr/supervisor/tests";
$critFile = "/home/l_tester/ptr/moduleDB/macros/criteria.dat";

@summaryFiles = `ls -1 $dbDir/$opt_m/summary_*.txt`;

$testFile     = "FullTest.root";
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

$moduleNr  = substr($mainD,0,5);
$testNr    = $subD;

$redTestNr = substr($testNr,2,3);
$sollTemp  = substr($testNr,1,3);

$meanT=-100.;
$sigmaT=-100.;

$meanC=-100.;
$sigmaC=-100.;
 
my $deadTot=0, $maskTot=0, $bumpsTot=0, $trimbitsTot=0, $addressTot=0, $badROCS=0, $badROCS1=0, $badROCS2=0, $rootTot=0;
my $badDEAD1=0, $badMASK1=0, $badBUMPS1=0, $badTRIMBITS1=0, $badADDRESS1=0;
my $badDEAD2=0, $badMASK2=0, $badBUMPS2=0, $badTRIMBITS2=0, $badADDRESS2=0;
my $mSCurve, $sSCurve, $mThr, $sThr, $mGain, $sGain, $mPed, $sPed;
my @SCurve, @Thr, @Gain, @Ped;
my $nSCurve=0, $nThr=0, $nGain=0, $nPed=0;

my $defB, $defC, $maskC, $curB, $curC, $sloB, $sloC;
my $noiA, $noiB, $trmB, $trmC, $gainB, $gainC, $pedB, $pedC;

&getGrading($critFile);

push(@SCurve, "<FONT COLOR=#009c66> SCurve ");
push(@Thr,    "<FONT COLOR=#9900ff> Vcal Thr ");
push(@Gain,   "<FONT COLOR=#3366ff> Gain ");
push(@Ped,    "<FONT COLOR=#ff9c33> Ped ");

if (-e "$dbDir/$opt_m/summaryTest.txt" ) {system("rm -f $dbDir/$opt_m/summaryTest.txt");}
open(SUMM, ">$dbDir/$opt_m/summaryTest.txt") || die "==> Cannot open $dbDir/$opt_m/summaryTest.txt\n";

if (-e "$dbDir/$opt_m/gradingTest.txt" ) {system("rm -f $dbDir/$opt_m/gradingTest.txt");}
open(GRAD, ">$dbDir/$opt_m/gradingTest.txt") || die "==> Cannot open $dbDir/$opt_m/gradingTest.txt\n";

# -- Loop over all summary files and extract relevant numbers from 

print "\nphpSummaryTest> test-Nr: $redTestNr and Soll-Temp: $sollTemp\n";

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

$mSCurve = $mSCurve/$nChips;
$sSCurve = $sSCurve/$nChips;
$mThr    = $mThr/$nChips;
$sThr    = $sThr/$nChips;
$mGain   = $mGain/$nChips;
$sGain   = $sGain/$nChips;
$mPed    = $mPed/$nChips;
$sPed    = $sPed/$nChips;

push(@SCurve, " ? </FONT>");
push(@Thr, " ? </FONT>");
push(@Gain, " ? </FONT>");
push(@Ped, " ? </FONT>");

&mkSummaryTest;
&checkCompletness("$dbDir/$opt_m", $moduleNr, $subD);

print "\nphpSummaryTest> $dbDir/$opt_m\/summaryTest.txt produced...\n";
print "\nphpSummaryTest> ................................................ finished\n\n";


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

	if (/nRootFileProblems/) {
	    s/nRootFileProblems: //g;
	    $root = $_;
	}
    }
      
    $deadTot += $dead;
    $maskTot += $mask;
    $bumps = $bumps - $dead;
    if($bumps<0) {$bumps = 0;}
    $bumpsTot += $bumps;
    $trimbits = $trimbits - 4*$dead;
    if($trimbits<0) {$trimbits = 0;}
    $trimbitsTot += $trimbits;
    $address = $address - $dead;
    if($address<0) {$address = 0;}
    $addressTot += $address;
    $rootTot += $root;

    $totalDef = $dead + $bumps + $trimbits + $address;

    if($dead>$defB     && $dead<$defC    ) { $badDEAD1++;     }
    if($mask>$defB     && $mask<$defC    ) { $badMASK1++;     }
    if($bumps>$defB    && $bumps<$defC   ) { $badBUMPS1++;    }
    if($trimbits>$defB && $trimbits<$defC) { $badTRIMBITS1++; }
    if($address>$defB  && $address<$defC ) { $badADDRESS1++;  }

    if($dead>$defC    ) { $badDEAD2++;     }
    if($mask>$defC    ) { $badMASK2++;     }
    if($bumps>$defC   ) { $badBUMPS2++;    }
    if($trimbits>$defC) { $badTRIMBITS2++; }
    if($address>$defC ) { $badADDRESS2++;  }

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
	    print "phpSummaryTest> TEMP $meanT $sigmaT\n";
	}

	if (/T-cycl/) {
	    s/T-cycl //g;
	    ($meanC, $sigmaC) = split(/ /, $_);
	    chop($sigmaC);
	    print "phpSummaryTest> CYCL $meanC $sigmaC\n";
	}

    }
}

#----------------------------------------------------------------------------------------

sub getFits() {
    my ($f) = @_;
    
    my $nSCurveChip, $mSCurveChip, $sSCurveChip, $nThrChip, $mThrChip, $sThrChip;
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

    } 
    
    # -------> Qualification parameters (only for comments, not grading yet!)
    
    if ( ($mSCurveChip > $noiB) || ($nSCurveChip < ($defB - 42)) )     {
    
	push(@SCurve, $roc); $nSCurve++; 
    }
    
    if ( (65*$sThrChip > $trmB) || ($nThrChip < ($defB - 42)) )     {
	
	push(@Thr, $roc); $nThr++; 
    }
	 
    if ($mGainChip != 0) {
	
	if ( ($sGainChip/$mGainChip > $gainB)  || ($nGainChip < ($defB - 42)) ) { 
	    
	    push(@Gain, $roc); $nGain++; 
	}
	
	if ( (65*$sPedChip > $pedB)  || ($nPedChip < ($defB - 42)) ) { 
	    
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
}

#----------------------------------------------------------------------------------------
sub mkSummaryTest() {

  print GRAD "DEAD $badDEAD1 $badDEAD2\n";
  print GRAD "MASK $badMASK1 $badMASK2\n";
  print GRAD "BUMPS $badBUMPS1 $badBUMPS2\n";
  print GRAD "TRIMBITS $badTRIMBITS1 $badTRIMBITS2\n";
  print GRAD "ADDRESS $badADDRESS1 $badADDRESS2\n";
 
  print SUMM "Directory $opt_m \n";
  print SUMM "ModuleNr TestNr $moduleNr $testNr\n";
  print SUMM "Defects $deadTot $maskTot $bumpsTot $trimbitsTot $addressTot \n";
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
  
  print SUMM "SCurve $mSCurve $sSCurve \n";
  print SUMM "Threshold   $mThr $sThr \n";
  print SUMM "Gain $mGain $sGain \n";
  print SUMM "Pedestal $mPed $sPed \n";

#comments
  if ($nSCurve > 0 ) { print SUMM "@SCurve\n"; }
  else { print SUMM "SCurves good\n"; }
  if ($nThr > 0 )   { print SUMM "@Thr\n"; }
  else { print SUMM "Thresholds good\n"; }
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

    print "\nphpSummaryTest> Checking completeness of directory $dir ($nChips chips)\n";

    open(MISS, ">$dir/missing.txt") || die "==> Cannot open $dir/missing.txt";
    print MISS "Missing: ";

    $ok = 1;
    $ic = 1;

    # directories
    if( !((-d "$dir/../T-10a") && (-d "$dir/../T-10b") && (-d "$dir/../T+17a")) ) { 

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

 
    #FullTest.root
    if( !(-e "$dir/FullTest.root") ) {

	$ok = 0; 
	print MISS "FullTest.root";
    }

    #Relevant Histograms in FullTest.root
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
 
    #iv.dat
    if( $temp ne "T-10a"&& !(-e "$dir/iv.dat") ) { 

	$ok = 0; 
	print MISS "iv ";
    }

    #moduleSummary - can not be checked at that moment !
#    if( !(-e "$dir/$mod$temp.gif") ) { 
#
#	$ok = 0; 
#	print MISS "$mod$temp.gif ";
#    }

    #dacParameters60_Cxx.dat
    $#dacs = -1;
    if ( (-e  "$dir/dacParameters60_C0.dat") || (-e  "$dir/dacParameters60_C8.dat")) {
	@dacs = `ls -1 $dir/dacParameters60_C*.dat`;
    }

    if( $#dacs < $nChips ) { 

	$ok = 0; 
	print MISS "dacParameters60 ";
    }

    #trimParameters60_Cxx.dat
    $#phcal = -1;
    if ( (-e  "$dir/trimParameters60_C0.dat") || (-e  "$dir/trimParameters60_C8.dat") ) {
	@phcal = `ls -1 $dir/trimParameters60_C*.dat`;
    }

    if( $#phcal < $nChips ) { 
	
	$ok = 0;
	print MISS "trimParameters60 ";
    }

    #SCurveData_Cxx.dat
    $#scurveD = -1;
    if ( (-e  "$dir/SCurveData_C0.dat") || (-e  "$dir/SCurveData_C8.dat") ) {
	@scurveD = `ls -1 $dir/SCurveData_C*.dat`;
    }

    if( $#scurveD < $nChips ) {

	$ok = 0;
	print MISS "SCurveData ";
    }

    #SCurve_Cxx.dat
    $#scurve = -1;
    if ( (-e  "$dir/SCurve_C0.dat") || (-e  "$dir/SCurve_C8.dat") ) {
	@scurve = `ls -1 $dir/SCurve_C*.dat`;
    }

    if( $#scurve < $nChips ) {

	$ok = 0;
	print MISS "SCurve ";
    }

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
	if ( $ic ) { print "\nphpSummaryTest> !!!!!!!!!  There are files missing in directory $dir !!! \n\n"; }
    }
    else {

        print "phpSummaryTest> ... complete ! \n";
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
    }
}
