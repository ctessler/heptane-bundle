#!/usr/bin/perl -w
use warnings;
use strict;
use Getopt::Long;
use vars qw/$HDRFMT $LINEFMT/;
Getopt::Long::Configure(qw{no_auto_abbrev no_ignore_case_always});

$HDRFMT ="%-11s%-3s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-8s%-8s\n";
$LINEFMT="%-11s%-3d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-8d%-8d\n";

sub usage {
	return "collect.pl -t <NTHREADS> <BENCHMARK NAME> ...\n" .
	    "\t-t\t\t\tthe number of threads the results were generated for\n" .
	    "\t-o|--output <FILE>\toutput file\n" .
	    "\t<BENCHMARK NAME>\ta converted benchmark\n";
}

# False entry point
# XXX-todo should handle @ARGV as @_ instead.
sub main {
	my ($threads, $ofname, $OFH);
	GetOptions("t=i", \$threads,
		   "output|o=s", \$ofname)
	    or die usage();
	die usage() if (!defined($threads));

	if (defined($ofname)) {
		open($OFH, '>', $ofname) or die "Could not open $ofname";
		select $OFH
	}

	my $ctxcost = get_ctx_cost(sprintf("%s/bundle/summary-%i.txt",
					   $ARGV[0], $threads));

	print_hdr($ctxcost);
	foreach my $mark (@ARGV) {
		my $resname = sprintf("%s/bundle/summary-%i.txt", $mark, $threads);
		proc_file($mark, $threads, $resname);
	}
	
	if (defined($ofname)) {
		select STDOUT;
		close($OFH)
	}
}
sub print_hdr {
	my $ctxcost = shift @_;
	print
	    "# Header Field Key:\n" .
	    "# All execution times include the thread level context switch costs\n" .
	    "# Thread level context switch cost: $ctxcost\n".
	    "#\tMARK\t Benchmark name\n" .
	    "#\tT\t Number of threads\n" .
	    "#\tHWCET\t Heptane WCET\n" .
	    "#\tI.1WO\t ILP v1.0 WCETO\n" .
	    "#\tI.2WO\t ILP v2.0 WCETO\n" .
	    "#\tH-I.2\t HWCET - I.2WO (the analytical benefit of BUNDLEP)\n" .
	    "#\tBEXE\t Execution time in cycles under BUNDLEP\n" .
	    "#\tHEXE\t Execution time in cycles under serialized execution\n" .
	    "#\tH-E\t HEXE - BEXE (the run time benefit of BUNDLEP)\n" .
	    "#\tCTXB\t Observed context switches for BUNDLEP\n" .
	    "#\tCTXM\t Maximum length of a single context switch for HEXE = BEXE\n";
	#               %-8s      %3s  %10s     %10s     %10s     %10s
	printf($HDRFMT, "# MARK", "T", "HWCET", "I.1WO", "I.2WO", "H-I.2",
        #               %-10s   %-10s   %-10s  %-8     %-8
	                "BEXE", "HEXE", "H-E", "CTXS", "CXTM");
	
}

sub proc_file {
	my ($mark, $threads, $file, $h, $dmatch);
	$dmatch='(\d+)\.{0,1}\d*';
	($mark, $threads, $file) = @_;
	open($h, '<', $file) or die "Could not open $file";
	my ($ctx, $i1wceto, $i2wceto, $hwcet, $bexe, $bctx, $hexe);
	while (my $line = <$h>) {
		my $regex = 'CTX:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$ctx = $1;
			next;
		}
		$regex = 'ILP 1.0 WCETO:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$i1wceto = $1;
			next;
		}
		$regex = 'ILP 2.0 WCETO:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$i2wceto = $1;
			next;
		}
		$regex = 'Heptane WCET:\s+' . "$dmatch" . '\s+\(1 thread\)';
		if ($line =~ /$regex/) {
			$hwcet = $1;
			next;
		}
		$regex = 'Bundle Observed Execution Time:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$bexe = $1;
			next;
		}
		$regex = 'Bundle Thread Level Switches:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$bctx = $1;
			next;
		}
		$regex = 'Serial Observed Execution Time:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$hexe = $1;
			next;
		}
	}
	foreach my $var ($ctx, $i1wceto, $i2wceto, $hwcet, $bexe, $bctx, $hexe) {
		die "Unable to process file $file" if !defined($var);
	}
	# Calculate the maximum CTX switch for the two methods to be equal
	my $slack = $hexe - $bexe;
	my $ctxm = $slack / $threads;
	
	# Incorporate CTX costs
	$hwcet = ($hwcet + $ctx) * $threads;
	$bexe += $ctx * $bctx;
	$hexe += $ctx * $threads;

	printf($LINEFMT, $mark, $threads, $hwcet, $i1wceto, $i2wceto,
	       $hwcet - $i2wceto, $bexe, $hexe, $hexe - $bexe, $bctx, $ctxm);
	
	close($h);
}

sub get_ctx_cost {
	my ($ctx, $file, $dmatch, $h);
	$dmatch='(\d+)\.{0,1}\d*';	
	$file = shift @_;
	open($h, '<', $file) or die "Could not open $file";
	while (my $line = <$h>) {
		my $regex = 'CTX:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$ctx = $1;
			last;
		}
	}
	close($h);
	die "Unable to open read $file" if (!defined($ctx));
	return $ctx;
}
main(@ARGV);
