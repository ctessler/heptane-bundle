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
	return "wcetoben-vs-sets.pl -c <MAX SIMULATOR CONFIG> -t <MAX THREADS>\n" .
	    "\t-c <LETTER>\t\t\t From a to <LETTER>\n" .
	    "\t-t <MAX THREADS>\t\t From 1 to <MAX THREADS> by powers of 2\n";
}

# False entry point
sub main {
	my ($simcfg, $threads, $outputfile, $OFH);
	Getopt::Long::GetOptionsFromArray(\@_,
					  "c=s", \$simcfg,
					  "t=i", \$threads,
					  "o=s", \$outputfile
	    ) or die usage();
	die usage() if (!defined($simcfg));

	my ($fdat, $fplot);
	if (!defined($outputfile)) {
		$fplot = $fdat = 'bundle-vs-hept';
	}
	$fdat .= '.dat';
	open(my $DH, '>', $fdat);
	# Print the header
	print $DH "# BUNDLE Benefit vs Architecture Configuration\n";
	print $DH "# \"Arch\"\t";
	for (my $t = 1; $t <= $threads; $t *= 2) {
		print $DH "\t$t:WECTO\t$t:EXE";
	}
	print $DH "\n";
	foreach my $sim ('a' .. $simcfg) {
		my ($brt, $cpi, $sets);
		for (my $t=1; $t <= $threads; $t *= 2) {
			my ($fname, $FH, $posexe, $poswceto);
			$fname = "results-sim-$sim.cfg-$t";
			open($FH, '<', $fname) or die "Couldn't open $fname";
			# Count the number of BUNDLEP wins 
			$posexe = $poswceto = 0;
			while (my $line = <$FH>) {
				my (@fields, $benexe, $benwceto);
				if ($line =~ /\#/) {
					next;
				}
				@fields = split(/\s+/, $line);
				if (!defined($brt)) {
					($brt, $cpi, $sets) = @fields[2,3,4];
					print $DH "\"($brt:$cpi, $sets)\"";
				} 
				($benwceto,$benexe) = @fields[8,11];
				if ($benwceto >= 1) {
					$poswceto++;
				}
				if ($benexe >= 1) {
					$posexe++;
				}
			}
			close($FH);
			print $DH "\t$poswceto\t$posexe";
		}
		print $DH "\n";
	}
	close($DH);

	my $terminal='set terminal epslatex standalone ", 10" header "\\\\usepackage{amssymb}"';
	my $xtics='set xtics rotate by 25 offset -4,-2';
	my $xlabel='set xlabel offset 0,-1.1';
	my $style='set style fill solid border -1';
	$style='set style fill pattern border';	

	my $PH;
	open($PH, '>', "$fplot-01.plot");
	(my $plot = qq{
	 $terminal
#	 set title "BUNDLEP WCETO Performance vs Heptane WCET" 
	 set style data histogram
	 set style histogram cluster gap 2
	 $style
	 set ylabel 'Count of Benchmarks where \${\\Delta_\\omega > 0}\$'
	 set xlabel 'Architecture (\${\\mathbb{B}}\$:\${\\mathbb{I}}\$, \${\\ell}\$)'
	 $xtics
	 $xlabel
	 plot 'data/$fdat' using 2:xtic(1) ti '\${m=1}\$' fillstyle pattern 0 fc rgb 1, \\
	 	'' u 4 ti '\${2}\$' fillstyle pattern 1 fc rgb 2, \\
	 	'' u 6 ti '\${4}\$' fillstyle pattern 2 fc rgb 4, \\
	 	'' u 8 ti '\${8}\$' fillstyle pattern 6 fc rgb 3, \\
	 	'' u 10 ti '\${16}\$' fillstyle pattern 3 fc rgb 5
	 }) =~ s/^\t//mg;
	print $PH $plot;
	close($PH);

	open($PH, '>', "$fplot-02.plot");
	$plot = qq{
	 $terminal
#	 set title "BUNDLEP Execution Performance vs Serial Execution"
	 set style data histogram
	 set style histogram cluster gap 1
	 $style
	 set ylabel 'Count of Benchmarks where \${\\Delta_B > 0}\$'
	 set xlabel 'Architecture (\${\\mathbb{B}}\$:\${\\mathbb{I}}\$, \${\\ell}\$)'
	 $xtics
	 $xlabel
	 plot 'data/$fdat' using 3:xtic(1) ti '\${m=1}\$' fillstyle pattern 0 fc rgb 1, \\
		'' u 5 ti '\${2}\$' fillstyle pattern 1 fc rgb 1, \\
		'' u 7 ti '\${4}\$' fillstyle pattern 2 fc rgb 4, \\
	 	'' u 9 ti '\${8}\$' fillstyle pattern 6 fc rgb 3, \\
	 	'' u 11 ti '\${16}\$' fillstyle pattern 3 fc rgb 5
	};
	$plot =~ s/^\t//mg;
	print $PH $plot;
	close($PH);
	
	return;
}
main(@ARGV);
