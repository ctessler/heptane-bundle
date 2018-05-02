#!/usr/bin/perl -w
use warnings;
use strict;
use Getopt::Long;
use Cwd;
use File::Basename;
use vars qw/$HDRFMT $LINEFMT $BDIR/;
Getopt::Long::Configure(qw{no_auto_abbrev no_ignore_case_always});

$BDIR=dirname(Cwd::abs_path($0));
print "BASEDIR $BDIR\n";

sub usage {
	return "exe-vs-sets.pl -c <MAX SIMULATOR CONFIG> -t <MAX THREADS>\n" .
	    "\t-c <LETTER>\t\t\t From a to <LETTER>\n" .
	    "\t-t <MAX THREADS>\t\t From 1 to <MAX THREADS> by powers of 2\n";
}

# False entry point
sub main {
	my ($simcfg, $threads, $OFH);
	Getopt::Long::GetOptionsFromArray(\@_,
			    "c=s", \$simcfg,
			    "t=i", \$threads)
	    or die usage();
	die usage() if (!defined($simcfg));

	my @bmarknames = get_names('results-sim-a.cfg-1');
	print "Benchmark names: " . join(", ", @bmarknames) . "\n";

	foreach my $bm (@bmarknames) {
		my $dfile = "$bm-02.dat";
		open(my $DH, '>', $dfile) or die "Couldn't open $dfile";
		print $DH "# $bm Configuration vs Benefit vs Thread Count\n";
		print $DH "\"Sim CFG\"";
		for (my $t = 1; $t <= $threads; $t *= 2) {
			print $DH "\t$t";
		}
		print $DH "\n";
		foreach my $sim ('a' .. $simcfg) {
			my ($brt, $cpi, $sets);
			for (my $t = 1; $t <= $threads; $t *= 2) {
				my (@fields, $sfile, @file, @match, $ben);
				$sfile = "results-sim-$sim.cfg-$t";
				open(my $SH, '<', $sfile) or die "Couldn't open $sfile";
				@file = <$SH>;
				@match = grep(/^$bm\W/, @file);
				die "Too many matches" if ($#match > 1);
				@fields = split(/\s+/,$match[0]);
				if (!defined($brt)) {
					($brt, $cpi, $sets) = @fields[2,3,4];
					print $DH "\"($brt:$cpi, $sets)\"";
				} 
				$ben = $fields[11];
				print $DH "\t$ben";
				close($SH);
			}
			print $DH "\n";
		}
		close($DH);

		my $pfile = "$bm-02.plot";
		(my $plot = qq{
		 set terminal epslatex standalone ", 8"
		 set title "Execution Benefit of BUNDLEP for $bm by Threads"
		 set style data histogram
		 set style histogram cluster gap 1
		 set style fill solid border -1
		 plot 'data/$dfile' using 2:xtic(1) ti col fc rgb 1, \\
		 	'' u 3 ti col fc rgb 2, \\
		 	'' u 4 ti col fc rgb 4, \\
		 	'' u 5 ti col fc rgb 8, \\
		 	'' u 6 ti col fc rgb 16
		 }) =~ s/^\t\t//mg;
		open(my $PH, '>', $pfile) or die "Couldn't open $pfile";
		print $PH $plot;
		close($PH);
	}
	return;
}
sub get_names {
	my (@names, $cfg);
	$cfg = shift;

	open(my $h, '<', $cfg) or die "Could not open $cfg";
	while(my $line = <$h>) {
		if ($line =~ /\#/) {
			next;
		}
		if ($line =~ /(\w+)\s+/) {
			push @names, $1;
		}
	}
	close($h);
	return @names;
}

main(@ARGV);
