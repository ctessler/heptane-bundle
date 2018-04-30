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

$HDRFMT ="%-11s%-3s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-8s%-8s\n";
$LINEFMT="%-11s%-3d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-8d%-8d\n";

sub usage {
	return "compare.pl -c <MAX SIMULATOR CONFIG> -t <MAX THREADS>\n" .
	    "\t-c <LETTER>\t\t\t From a to <LETTER>\n" .
	    "\t-t <MAX THREADS>\t\t From 1 to <MAX THREADS> by powers of 2\n" .
	    "\t-o|--output <FILE>\toutput file\n" .
	    "\t<RESULT FILE>\tthe results summary\n";
}

# False entry point
# XXX-todo should handle @ARGV as @_ instead.
sub main {
	my ($simcfg, $ofname, $threads, $OFH);
	GetOptions("c=s", \$simcfg,
		   "t=i", \$threads,
		   "output|o=s", \$ofname)
	    or die usage();
	die usage() if (!defined($simcfg));

	foreach my $cfg ('a' .. $simcfg) {
		my $ofname = "$cfg-compare.txt";
		open($OFH, '>', $ofname) or die "Couldn't open $ofname";
		select($OFH);

		print_hdr("$BDIR/sim-$cfg.cfg");
		for (my $t = 1; $t <= $threads; $t *= 2) {
			open(my $RES, '<', "results-sim-$cfg.cfg-$t")
			    or die "Could not open results-sim-$cfg.cfg-$t";
			print "\n# Threads: $t\n";
			while (my $line = <$RES>) {
				if ($line !~ /\# MARK/) {
					next;
				}
				print $line;
				last;
			}
			print <$RES>;
			close($RES);
		}

		select(STDOUT);
		close($OFH);
	}
}

sub get_ctx_cost {
	my ($ctx, $file, $dmatch, $h);
	$dmatch='(\d+)\.{0,1}\d*';	
	$file = shift @_;
	open($h, '<', $file) or die "Could not open $file";
	while (my $line = <$h>) {
		my $regex = 'Thread level context switch cost:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$ctx = $1;
			last;
		}
	}
	close($h);
	die "Unable to open read $file" if (!defined($ctx));
	return $ctx;
}

sub print_hdr {
	my ($simcfg, $dmatch, $h);
	$simcfg = shift @_;
	$dmatch='(\d+)\.{0,1}\d*';
	open($h, '<', $simcfg) or die "Could not open $simcfg";
	my ($brt, $exe, $latency, $assoc, $sets, $line_size);
	while (my $line = <$h>) {
		my $regex = 'latency =\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$latency = $1;
			next;
		}
		$regex = 'ways =\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$assoc = $1;
			next;
		}
		$regex = 'Sets:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$sets = $1;
			next;
		}
		$regex = 'line_size =\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$line_size = $1;
			next;
		}
		$regex = 'Block Reload Time.*:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$brt = $1;
			next;
		}
		$regex = 'hit.*:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$exe = $1;
			next;
		}

	}
	foreach my $var ($latency, $assoc, $sets, $line_size, $brt, $exe) {
		die "Unable to process file $simcfg" if !defined($var);
	}

	print "# BRT: $brt\t EXE: $exe\n";
	print "# Cache Parameters\n";
	print "# Ways\tSets\tSize(bytes)\n";
	print "# $assoc\t$sets\t$line_size\n";
}

sub proc_file {
}

main(@ARGV);
