#! /usr/bin/perl
#
# Usage: htmlSummaryPage.pl -m M7690-17-2 
#
# ----------------------------------------------------------------------

use Getopt::Std;
getopts('m:');

 
# --- Sort it ...
# FIXME!! foreach $file (@summaryFiles) {

$tmp = $opt_m;
$tmp =~ s/\.\.//;

@mtmp = split(/\//,$tmp);
$testNr=$mtmp[$#mtmp];  
@ntmp = split(/\-/,$mtmp[$#mtmp-1]);
$moduleNr=$ntmp[0];
$name= $moduleNr."".$testNr;

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
    my $nGainChip, $mGainChip, $sGainChip, $nPedChip, $mPedChip, $sPedChip;
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
	if (/Par1/) {
	    s/Par1 //g;
	    ($nPar1Chip, $mPar1Chip, $sPar1Chip) = split(/ /);
	    chop($sPar1Chip);
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
    if($dead>42 || $mask>42 || $bumps>42 || $trimbits>42 || $address>42) { $badROCS++; } 

    # -- Make sure the following is consistent with the order defined in mkHeader()

 print HTML "	<TBODY>\n";
 print HTML "		<TR>\n";

 print HTML "			<TD HEIGHT=21 ALIGN=LEFT><FONT FACE=\"Bitstream Vera Serif\" SIZE=3 COLOR=\"#0000FF\"><A HREF=\"C$roc.gif\">C$roc</A></FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$dead</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$mask</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$bumps</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$trimbits</FONT></TD>\n";
 print HTML "			<TD ALIGN=CENTER><FONT FACE=\"Bitstream Vera Serif\" SIZE=3>$address</FONT></TD>\n";
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
 print HTML "	<COLGROUP><COL WIDTH=86></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=86></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=86></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=94></COLGROUP>\n";
 print HTML "	<COLGROUP><COL WIDTH=91></COLGROUP>\n";

 print HTML "	<TBODY>\n";
 print HTML "		<TR>\n";
 print HTML "			<TD WIDTH=114 HEIGHT=19 ALIGN=LEFT><FONT FACE=Bitstream Vera Serif SIZE=3 COLOR=#FF0000><a href=\"$name.gif\">$name</a></FONT></TD>\n";
 print HTML "			<TD WIDTH=86 ALIGN=LEFT><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=86 ALIGN=LEFT><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=86 ALIGN=LEFT><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=94 ALIGN=LEFT><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 print HTML "			<TD WIDTH=91 ALIGN=LEFT><FONT FACE=Bitstream Vera Serif><BR></FONT></TD>\n";
 
 print HTML "		</TR>\n";
 print HTML "	</TBODY>\n";
 print HTML "	<TBODY>\n";
 print HTML "		<TR>\n";
 print HTML "			<TD HEIGHT=21 ALIGN=LEFT><B><FONT FACE=Bitstream Vera Serif SIZE=3>ROC</FONT></B></TD>\n";
 print HTML "			<TD ALIGN=LEFT><B><FONT FACE=Bitstream Vera Serif SIZE=3>Dead</FONT></B></TD>\n";
 print HTML "			<TD ALIGN=LEFT><B><FONT FACE=Bitstream Vera Serif SIZE=3>Mask</FONT></B></TD>\n";

 print HTML "			<TD ALIGN=LEFT><B><FONT FACE=Bitstream Vera Serif SIZE=3>Bumps</FONT></B></TD>\n";
 print HTML "			<TD ALIGN=LEFT><B><FONT FACE=Bitstream Vera Serif SIZE=3>TrimBits</FONT></B></TD>\n";
 print HTML "			<TD ALIGN=LEFT><B><FONT FACE=Bitstream Vera Serif SIZE=3>Address</FONT></B></TD>\n";
 print HTML "		</TR>\n";
 print HTML "	</TBODY>\n";

}


# ----------------------------------------------------------------------
sub mkTrailer() {

 print HTML "</TABLE>\n";
 print HTML "</BODY>\n";

 print HTML "</HTML>\n";
}

