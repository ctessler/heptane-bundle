#!/usr/bin/perl -w
use warnings;
use strict;
use Getopt::Long;
use vars qw/$HDRFMT $LINEFMT/;
Getopt::Long::Configure(qw{no_auto_abbrev no_ignore_case_always});
$HDRFMT  =      "%-11s". "%-3s%-4s%-3s" ."%-3s%-10s" ."%-10s";
$HDRFMT .=      "%-10s". "%-10s" ."%-10s"."%-10s"."%-10s". "%-6s". "%-8s%-4s%-3s\n";
$LINEFMT =      "%-11s". "%-3d%-4d%-3d" ."%-3d%-10d" ."%-10d";
$LINEFMT.=      "%-10d". "%-10d" ."%-10d"."%-10d"."%-10d". "%-6d". "%-8d%-4d%-3d\n";

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

	if (defined($ofname)) {
		open($OFH, '>', $ofname) or die "Could not open $ofname";
		select $OFH
	}

	my ($bx, $tx);
	($bx, $tx) = get_ctx_cost(sprintf("%s/bundle/summary.txt", $ARGV[0]));

	print_hdr($bx, $tx);
	foreach my $mark (@ARGV) {
		my $resname = sprintf("%s/bundle/summary.txt", $mark);
		proc_file($mark, $resname);
	}
	
	if (defined($ofname)) {
		select STDOUT;
		close($OFH)
	}
}
sub print_hdr {
	my ($bx, $tx);
	($bx, $tx) = @_;
	print
	    "# Header Field Key:\n" .
	    "# All execution times include the thread level context switch costs\n" .
	    "#\tMARK\t Benchmark name\n" .
	    "#\tT\t Number of threads\n" .
	    "#\tB\t Block Reload Time\n" .
	    "#\tI\t CPI\n" .
	    "#\tS\t Number of Cache Sets\n" .
	    "#\tHWCET\t Heptane WCET (W/ CTX)\n" .
	    "#\tI.CWO\t Current ILP WCETO\n" .
	    "#\tI.PWO\t Previous ILP WCETO\n" .
	    "#\tH-I.C\t HWCET - I.CWO (the analytical benefit of BUNDLEP)\n" .
	    "#\tHEXE\t Heptane execution time including CTX switches\n" .	    
	    "#\tBEXE\t BUNDLEP execution time including CTX switches\n" .
	    "#\tHE-BE\t HEXE - BEXE (the run time benefit of BUNDLEP)\n" .
	    "#\tCTXB\t Observed bundle level context switches for BUNDLEP\n" .
	    "#\tCTXT\t Observed thread level context switches for BUNDLEP\n" .
	    "#\tBX\t Cycles per BUNDLE level context switch\n" .
	    "#\tTX\t Cycles per thread level context switch\n";
	printf($HDRFMT, "# MARK", "T", "B", "I", "S", "HWECT", "I.CWO",
	                "I.PWO", "H-I.C", "HEXE", "BEXE", "HE-BE", "CTXB", "CTXT",
	                "BX", "TX");
}

sub proc_file {
	my ($mark, $file, $h, $dmatch);
	$dmatch='(\d+)\.{0,1}\d*';
	($mark, $file) = @_;
	open($h, '<', $file) or die "Could not open $file";
	my ($threads, $brt, $cpi, $sets, $hwcet, $ilpc, $ilpp, $hexe, $bexe, $ctxb, 
	    $ctxt, $bx, $tx);
	while (my $line = <$h>) {
		my $regex = 'CACHE BRT:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$brt = $1;
			next;
		}
		$regex = 'THREADS:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$threads = $1;
			next;
		}
		$regex = 'CACHE CPI:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$cpi = $1;
			next;
		}
		$regex = 'CACHE SETS:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$sets = $1;
			next;
		}
		$regex = 'Hept. WCET\(\d+ \+ CTXS\):\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$hwcet = $1;
			next;
		}
		$regex = 'ILP WCETO \(Current\):\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$ilpc = $1;
			next;
		}
		$regex = 'ILP WCETO \(Previous\):\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$ilpp = $1;
			next;
		}
		$regex = 'Serial EXE\(W/ CTX\):\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$hexe = $1;
			next;
		}
		$regex = 'BUNDLE EXE\(W/ CTX\):\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$bexe = $1;
			next;
		}
		$regex = 'N BUNDLE CTXs:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$ctxb = $1;
			next;
		}
		$regex = 'N THREAD CTXs:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$ctxt = $1;
			next;
		}
		$regex = 'BUNDLE CTX:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$bx = $1;
			next;
		}
		$regex = 'THREAD CTX:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$tx = $1;
			next;
		}
	}
	foreach my $var ($brt, $cpi, $sets, $hwcet, $ilpc, $ilpp, $hexe, $bexe,
			 $ctxb, $ctxt, $bx, $tx) {
	    die "Unable to process file $file" if !defined($var);
	}
	printf($LINEFMT, $mark, $threads, $brt, $cpi, $sets, $hwcet, $ilpc, $ilpp,
	       $hwcet - $ilpc, $hexe, $bexe, $hexe - $bexe, $ctxb, $ctxt, $bx, $tx);
	
	close($h);
}

sub get_ctx_cost {
	my ($bx, $tx, $file, $dmatch, $h);
	$dmatch='(\d+)\.{0,1}\d*';	
	$file = shift @_;
	open($h, '<', $file) or die "Could not open $file";
	while (my $line = <$h>) {
		my $regex = 'BUNDLE CTX:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$bx = $1;
			next;
		}
		$regex = 'THREAD CTX:\s+' . "$dmatch";
		if ($line =~ /$regex/) {
			$tx = $1;
			next;
		}
	}
	close($h);
	die "Unable to open read $file" if (!defined($bx));
	die "Unable to open read $file" if (!defined($tx));
	return ($bx, $tx);
}
main(@ARGV);
