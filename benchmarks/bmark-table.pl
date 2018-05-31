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

	my $bmarkfile = "bm-names.tex";
	open(my $DH, '>', $bmarkfile) or die "Couldn't open $bmarkfile";	
	print $DH '  \begin{tabular}{|c|c|}' . "\n";
	print $DH '    \hline' . "\n";
	my $count=0;
	for my $bm (@bmarknames) {
		$count++;
		if ($count % 2 == 0) {
			print $DH "$bm";
			print $DH "\t" . '\\\\ \hline' . " \n";
			next;
		}
		print $DH "    $bm\t" . '& ';
	}
	print $DH '  \end{tabular}' . "\n";		
	close($DH);
	
	my $dfile = "bm-graphs.tex";
	open($DH, '>', $dfile) or die "Couldn't open $dfile";
	foreach my $bm (@bmarknames) {
		print $DH '\begin{figure*}[!ph]' . "\n";
		print $DH '\center' . "\n";
		print $DH '\begin{subfigure}{.5\textwidth}' . "\n";
		print $DH '\includegraphics[width=\linewidth]{' . $bm . '-01}' . "\n";
#		print $DH '\caption{Analytical Benefit of \texttt{BUNDLEP}}' . "\n";
		print $DH '\end{subfigure}~' . "\n";
		print $DH '\begin{subfigure}{.5\textwidth}' . "\n";
		print $DH '\includegraphics[width=\linewidth]{' . $bm . '-02}' . "\n";
#		print $DH '\caption{Run-Time Benefit of \texttt{BUNDLEP}}' . "\n";
		print $DH '\end{subfigure}~' . "\n";
		print $DH '\end{figure*}' . "\n";
		print $DH "\n";
	}
	close($DH);
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
