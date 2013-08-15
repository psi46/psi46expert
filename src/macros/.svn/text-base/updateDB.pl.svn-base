#! /usr/bin/perl

# Usage: ./updateDB -[t,r,h,s] -l [1,2,3,4,5]
#        ./updateDB -[t,r,h,s] -l [1,2,3,4,5] -p   for a dry-run
#
# Default: runs web-update for testes modules, half-modules and retests with -mw option
#
# options :  -t update all module tests 
#            -r update all module retests  
#            -h update all half-module tests  
#            -s update all short-tests  
#            -l 1 web-update with -w option
#            -l 2 web-update with -mw option
#            -l 3 web-update with -cmw option
#            -l 4 web-update with -s option
#            -l 5 web-update with -p option (default: 3)
#            -l 5 -P 0,1,2,3 web-update with -p option 
# -----

#----adjust to your setup ----------------------

$baseDir     = "/home/l_tester/ptr/moduleDB";
$macros      = "$baseDir/macros";

#----adjust to your setup ----------------------

use FileHandle;

use Getopt::Std;
getopts('l:f:P:thrsp');

my $type;

$#bookFiles = -1;
$#option = -1;

if ( !$opt_l ) {

    $opt_l = 2;
}

if ( $opt_t ) {

    push(@bookFiles, "module-tests.dat");
    push(@option, "");
    push(@type, "");
}
elsif ( $opt_h ) {

    push(@bookFiles, "half-module-tests.dat");
    push(@option, "\-h");
    push(@type, "a");
}
elsif ( $opt_r ) {

    push(@bookFiles, "retested-modules.dat");
    push(@option, "\-r");
    push(@type, "");
}
elsif ( $opt_s ) {

    push(@bookFiles, "shortTests.dat");
    push(@option, "\-z");
    push(@type, "");
}
else {

    push(@bookFiles, "module-tests.dat");
    push(@option, "");
    push(@type, "");
    push(@bookFiles, "half-module-tests.dat");
    push(@option, "\-h");
    push(@type, "a");
    push(@bookFiles, "retested-modules.dat");
    push(@option, "\-r");
    push(@type, "");
}


chdir("$baseDir");

foreach $cnt ( 0 .. $#bookFiles ) {
  
    $#upd = -1;
    $#upd2 = -1;
    $#ListOfDir = -1;
    $#ListOfDir2 = -1;

    $#modules = -1;

    &getListOfDir(@bookFiles[$cnt], $opt_h, $opt_s);
    @modules = @ListOfDir;
    print "@modules\n";
        
    foreach $dir (@modules) {
	
	chop($dir);

	if ($opt_s) { $dir = "shortTests/$dir"; }
	
	
	if( -d "$dir/T-10a" && !(-e "$dir/T+17a/comment_4.txt") ) { push(@upd, "$dir/T-10a"); }
	if( -d "$dir/T-10b" && !(-e "$dir/T+17b/comment_4.txt") ) { push(@upd, "$dir/T-10b"); }
	if( -d "$dir/T+17a" && !(-e "$dir/T+17a/comment_4.txt") ) { push(@upd, "$dir/T+17a"); }
    }
	
    if ( $opt_l == 1 )  { print " ./web_upd.pl $option[$cnt] $type[$cnt] -w @upd\n"; }
    if ( $opt_l == 2 )  { print " ./web_upd.pl $option[$cnt] $type[$cnt] -mw @upd\n"; }
    if ( $opt_l == 3 )  { print " ./web_upd.pl $option[$cnt] $type[$cnt] -cmw @upd\n"; }
    if ( $opt_l == 4 )  { print " ./web_upd.pl $option[$cnt] $type[$cnt] -s @upd\n"; }
    if ( $opt_l == 5 )  { 

	if ( !$opt_P ) {

	    print " ./web_upd.pl $option[$cnt] $type[$cnt] -p @upd\n"; 
	}
	else {

	    print " ./web_upd.pl $option[$cnt] $type[$cnt] -P $opt_P @upd\n"; 
	}
    }
   

    if ( !$opt_p ) {

	if ( $opt_l == 1 )  { system("./web_upd.pl $option[$cnt] $type[$cnt] -w @upd"); }
	if ( $opt_l == 2 )  { system("./web_upd.pl $option[$cnt] $type[$cnt] -mw @upd"); }
	if ( $opt_l == 3 )  { system("./web_upd.pl $option[$cnt] $type[$cnt] -cmw @upd"); }
	if ( $opt_l == 4 )  { system("./web_upd.pl $option[$cnt] $type[$cnt] -s @upd"); }
	if ( $opt_l == 5 )  { 
	    
	    if ( !$opt_P ) {
		
		system("./web_upd.pl $option[$cnt] $type[$cnt] -p @upd"); 
	    }
	    else {
		system("./web_upd.pl $option[$cnt] $type[$cnt] -P $opt_P @upd"); 
	    }
	}
    }


    if ( $#ListOfDir2 >= 0 ) {

	@modules = @ListOfDir2;
	print "@modules\n";
        
	foreach $dir (@modules) {
	    
	    chop($dir);
	    
	    if( -d "$dir/T-10a") { push(@upd2, "$dir/T-10a"); }
	    if( -d "$dir/T-10b") { push(@upd2, "$dir/T-10b"); }
	    if( -d "$dir/T+17a") { push(@upd2, "$dir/T+17a"); }
	}

	if ( $opt_l == 1 )  { print " ./web_upd.pl $option[$cnt] b -w @upd2\n"; }
	if ( $opt_l == 2 )  { print " ./web_upd.pl $option[$cnt] b -mw @upd2\n"; }
	if ( $opt_l == 3 )  { print " ./web_upd.pl $option[$cnt] b -cmw @upd2\n"; }
	if ( $opt_l == 4 )  { print " ./web_upd.pl $option[$cnt] b -s @upd2\n"; }
	if ( $opt_l == 5 )  { 
	    
	    if ( !$opt_P ) {
		
		print " ./web_upd.pl $option[$cnt] b -p @upd2\n"; 
	    }
	    else {
		
		print " ./web_upd.pl $option[$cnt] b -P $opt_P @upd2\n"; 
	    }
	}



	if ( !$opt_p ) {

	    if ( $opt_l == 1 )  { system("./web_upd.pl $option[$cnt] b -w @upd2"); }
	    if ( $opt_l == 2 )  { system("./web_upd.pl $option[$cnt] b -mw @upd2"); }
	    if ( $opt_l == 3 )  { system("./web_upd.pl $option[$cnt] b -cmw @upd2"); }
	    if ( $opt_l == 4 )  { system("./web_upd.pl $option[$cnt] b -s @upd2"); }
	    if ( $opt_l == 5 )  { system("./web_upd.pl $option[$cnt] b -p $opt_p @upd2"); }
	    if ( $opt_l == 5 )  { 
		
		if ( !$opt_P ) {
		    
		    system("./web_upd.pl $option[$cnt] b -p @upd2"); 
		}
		else {
		    system("./web_upd.pl $option[$cnt] b -P $opt_P @upd2"); 
		}
	    }
	    
	}
    }
}


#----------------------------------------------------------------------------------------

sub getListOfDir() {
    my ($file, $half, $short) = @_;

    chdir("$baseDir");

    $f = "book-keeping/$file";
    open(GR, "$f") || die "Cannot open $f\n"; 

    print "\n--> Reading module directories from $f\n";

    while (<GR>) {

	@col = split(/\ /,$_);
	if ($#col > 2 ) {
	    
	    if ( @col[0] eq substr(@col[2], 0, 5) ) {

		if ( $half ) {

		    $type = "";
		    &getHalfModuleType($col[2], $short);

		} else {

		    $type = "";
		}



		if ( $type eq "b" )      { 

		    push(@ListOfDir2, "@col[2]\n"); 
		
		} else { 

		    push(@ListOfDir, "@col[2]\n"); 
		}

	    }
	}
    }
	
    close(GR);
}

#----------------------------------------------------------------------------------------

sub getHalfModuleType() {
    my ($modNumber, $short) = @_;


    $type = "";
    if ( $short ) {

	@summaryFiles = `ls -1 shortTests/$modNumber/T?1??/summaryTest.txt`;

    } else {

	@summaryFiles = `ls -1 $modNumber/T?1??/summaryTest.txt`;
    }

    $summaryFile = @summaryFiles[0];
    chop($summaryFile);

    if ( -e "$summaryFile" ) {

	open(IN, "$summaryFile") || die "Cannot open $summaryFile\n";
	
	while (<IN>) {
	    
	    if (/Half-Module/) {

		s/Half-Module //g;
		$type = $_;
		chop($type);
	    }
	}
	
    }
}
