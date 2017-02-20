#!/usr/bin/perl -w

# A very simple script to do a quick validation on a mask file by
# looking at the active area, to make sure it's consistent across
# all ROCs.

use strict;

my $infile = shift;
open INFILE, $infile or die "Couldn't open $infile: $!\n";

# list keyed by: channel number, roc number, and then
# 0: x left edge, 1: x right edge, 2: y bottom edge, 3: y top edge
my @activeArea;

while (my $line = <INFILE>) {
    next if ($line =~ /^\#/);  # skip comments
    next if ($line =~ /^\s*$/); # skip blank lines
    next if ($line !~ /-/);    # if line doesn't contain a range, it's just
    # a single masked pixel, so ignore it

    my @fields = split(" ", $line);
    my $channel = 1 + 12*(8-$fields[0]) + 6*($fields[1]-1) +
	($fields[2]-5)/8;
    if ($fields[2] >= 21) { $channel++; }
    my $roc = $fields[3];
    #print "Processing line $channel $roc $fields[4] $fields[5]\n";
    if ($fields[5] eq "0-79") {
	if ($fields[4] =~ /^0-(\d+)/) {
	    $activeArea[$channel][$roc][0] = $1+1;
	}
	if ($fields[4] =~ /(\d+)-51/) {
	    $activeArea[$channel][$roc][1] = $1-1;
	}
    }
    if ($fields[4] eq "0-51") {
	if ($fields[5] =~ /^0-(\d+)/) {
	    $activeArea[$channel][$roc][2] = $1+1;
	}
	if ($fields[5] =~ /(\d+)-79/) {
	    $activeArea[$channel][$roc][3] = $1-1;
	}
    }
}

for (my $i=1; $i<24; ++$i) {
    next if (!$activeArea[$i]);
    for (my $r=0; $r<3; ++$r) {
	#print "Ch $i ROC $r: x: ".$activeArea[$i][$r][0]."-".$activeArea[$i][$r][1]." y: ".$activeArea[$i][$r][2]."-".$activeArea[$i][$r][3];
	my $xsize = $activeArea[$i][$r][1] - $activeArea[$i][$r][0] + 1;
	my $ysize = $activeArea[$i][$r][3] - $activeArea[$i][$r][2] + 1;
	print "Ch $i ROC $r: $xsize by $ysize\n";
    }
}

