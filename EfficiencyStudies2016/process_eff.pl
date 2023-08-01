#!/usr/bin/perl -w

use strict;

open INFILE, "efficiency_linearity_2016_v1.csv" or die "Input file not found";

my $line = <INFILE>; # skip header

my %eff;
my %efferr;

while (my $line = <INFILE>) {
    chomp $line;
    my @fields = split(",", $line);
    if ($fields[2] != 4 || $fields[3] != 1) { next; }
    if ($eff{$fields[7]}) { next; }

    my $fill = $fields[7];
    my $eff = $fields[6];
    my $efferr = $fields[9];

    $eff{$fill} = $eff;
    $efferr{$fill} = $efferr;
}

foreach my $fill (sort keys %eff) {
    print $fill, ",", $eff{$fill}, ",", $efferr{$fill}, "\n";
}
