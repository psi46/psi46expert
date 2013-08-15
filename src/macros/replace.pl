
open(INFILE, $ARGV[0]);

my $a = 2;

open(OUTFILE, ">".$ARGV[0].$a);

while (<INFILE>) {

	$_ =~ s/$ARGV[1]/$ARGV[2]/;
	print OUTFILE $_;
}

close INFILE;
close OUTFILE;

$cmd = "mv $ARGV[0]$a $ARGV[0]";
if(system($cmd)) { print "rename failed\n"; }