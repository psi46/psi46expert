#! /usr/bin/perl
#
# Usage: htmlSummaryPage.pl -m M7690-17-2 
#
# ----------------------------------------------------------------------

use Getopt::Std;
getopts('m:');

my $deadTot=0, $maskTot=0, $bumpsTot, $trimbitsTot, $addressTot, $gainTot=0, $pedTot=0, $par1Tot=0;

my $deadMax, $maskMax, $par1Max, $gainMax, $pedMax, $mP1Max, $sP1Max, $rGMax, $mGMin, $sGMax, $rPMax, $mPMax, $sPMax;

$deadMax = 0; 
$maskMax = 0;
$par1Max = 0;
$gainMax = 0;
$pedMax  = 0;
$mP1Max  = 0;
$sP1Max = 0; 
$rGMax  = 0;
$mGMin  = 10000;
$sGMax  = 0;
$rPMax  = 0;
$mPMax  = 0;
$sPMax  = 0;
 
# --- Sort it ...
# FIXME!! foreach $file (@summaryFiles) {

$tmp = $opt_m;
$tmp =~ s/\.\.//;

@mtmp = split(/\//,$tmp);
$testNr=$mtmp[$#mtmp];  
@ntmp = split(/\-/,$mtmp[$#mtmp-1]);
$moduleNr=$ntmp[0];
$name= $moduleNr."".$testNr;

if ( $testNr eq "T-10a" ) {

    $otherTemp = "T+17a";
}

if ( $testNr eq "T+17a" ) {

    $otherTemp = "T-10a";
}

$oth = "http://kamor.ethz.ch/moduleTests/moduleDB/shortTests/".$moduleNr."/".$otherTemp."/".$moduleNr.$otherTemp.".html";

$dirName = $opt_m;
$dirName =~ s/\.\.\///;

open(HTML, ">$opt_m/$name.html") || die "==> Cannot open $opt_m/$name.html\n";


&mkHeader;

# -- Loop over all summary files and extract relevant numbers from them

@summaryFiles1 = `ls -1 $opt_m/summary_C?.txt`;


foreach $file (@summaryFiles1) {

    chop($file);
    printRow($file);
    

}

if ( -e "$opt_m/summary_C10.txt") {

    @summaryFiles2 = `ls -1 $opt_m/summary_C??.txt`;
    
    foreach $file (@summaryFiles2) {
	
	chop($file);
	printRow($file);
    }
}

&mkTrailer;

close(HTML);


# ----------------------------------------------------------------------
sub printRow() {
    my ($f) = @_;
    
    my $roc, $dead, $mask, $bumps, $trimbits, $address;
    my $nGainChip, $mGainChip, $sGainChip, $rGainChip, $nPedChip, $mPedChip, $sPedChip, $rPedChip;
    my $nPar1Chip, $mPar1Chip, $sPar1Chip;
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
    
    $dead /= 1; $bumps /= 1; $trimbits /= 1; $address /= 1; $gain /= 1; $ped /= 1; $par1 /= 1;

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
    $gainTot += $gain;
    $pedTot += $ped;
    $par1Tot += $par1;
    $rootTot += $root;

    if($dead>42 || $mask>42 || $bumps>42 || $trimbits>42 || $address>42 || $gain>42 || $ped>42 || $par1>42) { 
	
	$badROCS++; 
    } 

    $rPedChip = 65.* $sPedChip;
    $rGainChip = 100*$sGainChip/$mGainChip;

    if ( $mPar1Chip < 100. && $mPar1Chip > 0.01 ) {
	$mP1 = sprintf("%.2f", $mPar1Chip);
    } else {
	$mP1 = sprintf("%.1e", $mPar1Chip);
    }

    if ( $sPar1Chip < 100. && $sPar1Chip > 0.001 ) {
	$sP1 = sprintf("%.3f", $sPar1Chip);
    } else {
	$sP1 = sprintf("%.2e", $sPar1Chip);
    }

    $nP1 = 4160 - $nPar1Chip;


    if ( $mGainChip < 100. && $mGainChip > 0.01 ) {
	$mG = sprintf("%.2f", $mGainChip);
    } else {
	$mG = sprintf("%.1e", $mGainChip);
    }

    if ( $sGainChip < 10. && $sGainChip > 0.001 ) {
	$sG = sprintf("%.3f", $sGainChip);
    } else {
	$sG = sprintf("%.2e", $sGainChip);
    }

    if ( $rGainChip < 100. && $rGainChip > 0.001 ) {
	$rG = sprintf("%.1f", $rGainChip);
    } else {
	$rG = sprintf("%.2e", $rGainChip);
    }

    $nG = 4160 - $nGainChip;


    if ( $mPedChip < 10000. && $mPedChip > 1. ) {
	$mP = sprintf("%.0f", $mPedChip);
    } else {
	$mP = sprintf("%.3e", $mPedChip);
    }

    if ( $sPedChip < 1000. && $sPedChip > 0.1 ) {
	$sP = sprintf("%.1f", $sPedChip);
    } else {
	$sP = sprintf("%.3e", $sPedChip);
    }

    if ( $rPedChip < 10000. ) {
	$rP = sprintf("%.0f", $rPedChip);
    } else {
	$rP = sprintf("%.3e", $rPedChip);
    }

    $nP = 4160 - $nPedChip;

    if ( $dead > $deadMax ) { $deadMax = $dead; }
    if ( $mask > $maskMax ) { $maskMax = $mask; }
    if ( $par1 > $par1Max ) { $par1Max = $par1; }
    if ( $gain > $gainMax ) { $gainMax = $gain; }
    if ( $ped > $pedMax ) { $pedMax = $ped; }
    if ( $mP1 > $mP1Max ) { $mP1Max = $mP1; }
    if ( $sP1 > $sP1Max ) { $sP1Max = $sP1; }
    if ( $rG > $rGMax ) { $rGMax = $rG; }
    if ( $mG < $mGMin ) { $mGMin = $mG; }
    if ( $sG > $sGMax ) { $sGMax = $sG; }
    if ( $rP > $rPMax ) { $rPMax = $rP; }
    if ( $mP > $mPMax ) { $mPMax = $mP; }
    if ( $sP > $sPMax ) { $sPMax = $sP; }

    # -- Make sure the following is consistent with the order defined in mkHeader()

 print HTML "	<TBODY>\n";
 print HTML "		<TR>\n";

 print HTML "			<TD HEIGHT=21 ALIGN=LEFT><FONT FACE=\"Bitstream Vera Serif\" SIZE=3 COLOR=\"#0000FF\"><A HREF=\"C$roc.gif\">C$roc</A></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$dead</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$mask</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$par1</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$gain</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$ped</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><B>$mP1</B></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$sP1</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><B>$rG</B></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$mG</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$sG</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><B>$rP</B></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$mP</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$sP</FONT></TD>\n";

 print HTML "		</TR>\n";
 print HTML "	</TBODY>\n";




}

# ----------------------------------------------------------------------
sub mkHeader() {

   print HTML "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n";

   print HTML "<HTML>\n";
   print HTML "<HEAD>\n";
	
   print HTML "	<META HTTP-EQUIV=CONTENT-TYPE CONTENT=text/html; charset=utf-8>\n";
   print HTML "	<TITLE>$opt_m</TITLE>\n";
   print HTML "	<META NAME=GENERATOR CONTENT=OpenOffice.org 1.1.1  (Linux)>\n";
   print HTML "	<META NAME=CREATED CONTENT=20051214;17285800>\n";
   print HTML "	<META NAME=CHANGED CONTENT=20051214;19021500>\n";
	
   print HTML "	<STYLE>\n";
   print HTML "		<!-- \n";
   print HTML "		BODY,DIV,TABLE,THEAD,TBODY,TFOOT,TR,TH,TD,P { font-family:Bitstream Vera Sans; font-size:x-small }\n";
   print HTML "		 -->\n";
   print HTML "	</STYLE>\n";

	
    print HTML "</HEAD>\n";

 print HTML "<BODY TEXT=#000000>\n";
 print HTML "<TABLE FRAME=BOX CELLSPACING=0 COLS=6 RULES=GROUPS BORDER=1>\n";
 print HTML "	<COLGROUP><COL WIDTH=114></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";

 print HTML "	<COLGROUP><COL WIDTH=90></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=90></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=90></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=84></COLGROUP>\n";

 print HTML "	<TBODY>\n";
 print HTML "		<TR>\n";
 print HTML "			<TD WIDTH=114 HEIGHT=19 ALIGN=LEFT><FONT FACE=Bitstream Vera Serif SIZE=3 COLOR=#FF0000><a href=\"$name.gif\">$name</a></FONT></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";

 print HTML "			<TD WIDTH=90 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";

 print HTML "			<TD WIDTH=90 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif SIZE=3><B>Rel.Gain Width</B></FONT></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";

 print HTML "			<TD WIDTH=90 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif SIZE=3><B>Ped.Spread</B></FONT></B></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=84 ALIGN=CENTER><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 
 print HTML "		</TR>\n";
 print HTML "	</TBODY>\n";
 print HTML "	<TBODY>\n";
 print HTML "		<TR>\n";
 print HTML "			<TD HEIGHT=21 ALIGN=LEFT><B><FONT FACE=Bitstream Vera Serif SIZE=3>ROC</FONT></B></TD>\n";
 print HTML "			<TD ALIGN=CENTER><B><FONT FACE=Bitstream Vera Serif SIZE=3>Dead</FONT></B></TD>\n";
 print HTML "			<TD ALIGN=CENTER><B><FONT FACE=Bitstream Vera Serif SIZE=3>Mask</FONT></B></TD>\n";

 print HTML "			<TD ALIGN=CENTER><B><FONT FACE=Bitstream Vera Serif SIZE=3>Par.1</FONT></B></TD>\n";
 print HTML "			<TD ALIGN=CENTER><B><FONT FACE=Bitstream Vera Serif SIZE=3>Gain</FONT></B></TD>\n";
 print HTML "			<TD ALIGN=CENTER><B><FONT FACE=Bitstream Vera Serif SIZE=3>Ped.</FONT></B></TD>\n";
 print HTML "			<TD ALIGN=CENTER><B><FONT FACE=Symbol SIZE=3>m</FONT><FONT FACE=Bitstream Vera Serif SIZE=3>(Par1)</B></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=Bitstream Vera Serif SIZE=3>RMS</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=Bitstream Vera Serif SIZE=3><B>[\%]</B></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=Symbol SIZE=3>m</FONT><FONT FACE=Bitstream Vera Serif SIZE=3>(Gain)</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=Bitstream Vera Serif SIZE=3>RMS</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=Bitstream Vera Serif SIZE=3><B>[el.]</B></FONT></B></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=Symbol SIZE=3>m</FONT><FONT FACE=Bitstream Vera Serif SIZE=3>(Ped)</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=Bitstream Vera Serif SIZE=3>RMS</FONT></TD>\n";
 print HTML "		</TR>\n";
 print HTML "	</TBODY>\n";

}


# ----------------------------------------------------------------------
sub mkTrailer() {
 print HTML "	<TBODY>\n";
 print HTML "		<TR>\n";

 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=1></FONT></TD>\n";

 print HTML "		</TR>\n";
 print HTML "	</TBODY>\n";
 print HTML "	<TBODY>\n";
 print HTML "		<TR>\n";

 print HTML "			<TD ALIGN=LEFT><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><B>check sheet</B></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I><B>$deadTot</B></I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I><B>$maskTot</B></I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I>$par1Tot</I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I>$gainTot</I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I>$pedTot</I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I><B>$mP1Max</B></I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I><B>$sP1Max</B></I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I>$rGMax</I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I>$mGMin</I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I><B>$sGMax</B></I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I>$rPMax</I></B></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I>$mPMax</I></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3><I><B>$sPMax</B></I></FONT></TD>\n";

 print HTML "		</TR>\n";
 print HTML "	</TBODY>\n";

 print HTML "</TABLE>\n";


 print HTML "<BR>\n";
 print HTML "<P ALIGN=LEFT WIDTH=114 HEIGHT=1>";
 print HTML "<FONT FACE=Bitstream Vera Serif SIZE=3 COLOR=#FF0000><a href=\"$oth\">switch to $otherTemp</a></FONT></P>";
 print HTML "<P ALIGN=RIGHT WIDTH=114 HEIGHT=1>";
 print HTML "<FONT FACE=Bitstream Vera Serif SIZE=3 COLOR=#FF0000><a href=\"http://kamor.ethz.ch/moduleTests/moduleDB/shortTests/prodTable.php\?Serie=recent\">back to main table</a></FONT></P>\n";

 print HTML "</BODY>\n";


 print HTML "</HTML>\n";
}

