#!/usr/bin/perl -w

# Like checkOnlineMaskSize.pl, but operates on the "offline" masks
# (with FED channel number and no ranges in the fields).

use strict;

my $infile = shift;
open INFILE, $infile or die "Couldn't open $infile: $!\n";

# list keyed by: channel number, roc number, and then
# 0: x left edge, 1: x right edge, 2: y bottom edge, 3: y top edge
my @activeArea;

while (my $line = <INFILE>) {
    next if ($line =~ /^\#/);  # skip comments
    next if ($line =~ /^\s*$/); # skip blank lines
    # next if ($line !~ /-/);    # if line doesn't contain a range, it's just
    # a single masked pixel, so ignore it

    my @fields = split(" ", $line);
    my $channel = $fields[0];
    my $roc = $fields[1];
    #print "Processing line $channel $roc $fields[4] $fields[5]\n";
    if ($fields[4] == 0 && $fields[5] == 79) {
	if ($fields[2] == 0) {
	    $activeArea[$channel][$roc][0] = $fields[3] + 1;
	}
	elsif ($fields[3] == 51) {
	    $activeArea[$channel][$roc][1] = $fields[2] - 1;
	} else {
	    warn "Arg!\n";
	}
    }
    if ($fields[2] == 0 && $fields[3] == 51) {
	if ($fields[4] == 0) {
	    $activeArea[$channel][$roc][2] = $fields[5] + 1;
	}
	elsif ($fields[5] == 79) {
	    $activeArea[$channel][$roc][3] = $fields[4] - 1;
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

