#!/usr/bin/perl -w

my $SECTION = $ARGV[0];
my $RUNLINE = $SECTION + 1;

my $EXEDIR       = "/data/PLT/DHidasPLT";
my $SLINKDIR     = "/data/PLT/Testbeam_Oct2011/slink";
my $GAINCALDIR   = "/data/PLT/Testbeam_Oct2011/GainCalFits_Converted";
my $ALIGNMENTDIR = "$EXEDIR/ALIGNMENT";
my $BASEOUTDIR   = "/afs/cern.ch/user/d/dhidas/public/html/PLT/temp/SeeOld2";
my $RUNLIST      = "$EXEDIR/scripts/RunList_October2011PS.dat";

my $THISRUNDIR = "$BASEOUTDIR/Running_$SECTION";
mkdir $THISRUNDIR;
chdir $THISRUNDIR;

# make sure you have a "plots" directory in the current dir
if (!-e "plots") {
  print "creating plots dir\n";
  mkdir "plots" or die "cannot create plots dir $!";
}

# make baseoutdir if it doesn't exist
if (!-e $BASEOUTDIR) {
  print "creating BASEOUTDIR: $BASEOUTDIR\n";
  mkdir $BASEOUTDIR or die "cannot create BASEOUTDIR: $BASEOUTDIR $!";
}



# open file for reading
#open RUNS, $RUNLIST or die "cannot open runlist: $RUNLIST $!";
my @MYRUNS =  `head -$RUNLINE $RUNLIST | tail -1`;

# this is for command you will run
my $command;

# loop over each line in the run file
my $line;
#foreach $line (<RUNS>) {
foreach $line (@MYRUNS) {

  # yum yum
  chomp $line;
  print "$line\n";

  # clean up plots dir
  `rm -rf plots/*.gif`;

  # split line into it's correct chuncks
  my @rundata  = split(/ +/, $line);
  my $run      = $rundata[0];
  my $file     = $rundata[1];
  my $calib    = $rundata[2];
  my $cassette = $rundata[3];
  my $hub      = $rundata[4];
  my $trigger  = $rundata[5];
  my $channel  = $rundata[6];
  my $nevents  = $rundata[7];

  # make directory for this run
  my $outdir = "$BASEOUTDIR/Run_$run" . "_Cassette$cassette" . "_Ch$channel" . "_roc$trigger";
  if (!-e $outdir) {
    mkdir $outdir or die "cannot make run dir: $outdir $!";
  }


  # run occupancy
  $command = "$EXEDIR/OccupancyPlots $SLINKDIR/$file";
  print "$command\n";
  `$command`;
  $command = "cp plots/Occupancy_All_Ch$channel.gif $outdir/Occupancy_1x1.gif";
  print "$command\n";
  `$command`;
  $command = "cp plots/Occupancy_Efficiency_Ch$channel.gif $outdir/Occupancy_Efficiency_1x1.gif";
  print "$command\n";
  `$command`;

  $command = "$EXEDIR/TrackingEfficiency $SLINKDIR/$file $GAINCALDIR/GainCalFits_$calib.dat $ALIGNMENTDIR/Alignment_Straight.dat";
  print "$command\n";
  `$command`;
  $command = "cp plots/TrackingEfficiencyMap_Ch$channel"."_ROC0.gif $outdir/TrackingEfficiencyMap_ROC0.gif";
  `$command`;
  $command = "cp plots/TrackingEfficiencyMap_Ch$channel"."_ROC1.gif $outdir/TrackingEfficiencyMap_ROC1.gif";
  `$command`;
  $command = "cp plots/TrackingEfficiencyMap_Ch$channel"."_ROC2.gif $outdir/TrackingEfficiencyMap_ROC2.gif";
  `$command`;

  $command = "$EXEDIR/PulseHeightsTrack $SLINKDIR/$file $GAINCALDIR/GainCalFits_$calib.dat $ALIGNMENTDIR/Alignment_Test.dat";
  print "$command\n";
  `$command`;
  $command = "cp plots/PulseHeightTrack_Ch$channel.gif $outdir/PulseHeightTrack.gif";
  `$command`;

  PrintHTML($outdir, $run, $cassette, $channel, $trigger, $file, $calib);


}

`rm -rf $THISRUNDIR`;

sub PrintHTML
{
  my ($outdir, $run, $cassette, $channel, $trigger, $file, $calib) = @_;

  open OUT, ">$outdir/index.html" or die "cannot open output html file $!";

print OUT "<html>";
print OUT "<body>";
print OUT "<h1>Run $run  Cassette $cassette  Ch$channel</h1>";
print OUT "<h2>Trigger on ROC $trigger</h2>";
print OUT "<p>File: $file<br>Calibration: $calib</p>";
print OUT << 'ENDOFHTML'
<hr>
<p>
Tracking efficiency maps.  Requires 2 single pixel clusters which extrapolate to plane under test.
We check to see if there is a cluster within 3 pixels of extrapolated point and calculate an
efficiency for extrapolated point.
</p>
<img width="200" src="TrackingEfficiencyMap_ROC0.gif">
<img width="200" src="TrackingEfficiencyMap_ROC1.gif">
<img width="200" src="TrackingEfficiencyMap_ROC2.gif">

<hr>
<p>
Occupancy with 1 pixel removed on the border
</p>
<img width="600" src="Occupancy_1x1.gif"><br>

<hr>
<p>
Relative occupancy to neighbors in a 3x3 box with 1 pixel on the border removed
</p>
<img width="600" src="Occupancy_Efficiency_1x1.gif"><br>

<hr>
<p>
Pulse heights..
</p>
<img width="600" src="PulseHeightTrack.gif"><br>
<hr>
</body>
</html>
ENDOFHTML

}
