#!/bin/bash

list2015=(gaincalfast_20150811.120552.avg.txt gaincalfast_20150923.225334.avg.txt gaincalfast_20151029.220336.avg.txt)
for l in ${list2015[*]}
do
 ./GainCalFastFits_old /localdata/2015/CALIB/GainCal/$l
done


list2016=(gaincalfast_20160404.161634.avg.txt  gaincalfast_20160526.125155.avg.txt  gaincalfast_20160703.144920.avg.txt  gaincalfast_20160819.113115.avg.txt
gaincalfast_20160501.155303.avg.txt  gaincalfast_20160614.081148.avg.txt  gaincalfast_20160727.172520.avg.txt  gaincalfast_20161007.162624.avg.txt)
for l in ${list2016[*]}
do
 ./GainCalFastFits_old /localdata/2016/CALIB/GainCal/$l
done

list2017=(gaincalfast_20170504.113854.avg.txt  gaincalfast_20170518.143905.avg.txt  gaincalfast_20170707.214121.avg.txt  gaincalfast_20170815.135532.avg.txt  gaincalfast_20171105.175146.avg.txt
gaincalfast_20170504.121153.avg.txt  gaincalfast_20170601.090534.avg.txt  gaincalfast_20170717.224815.avg.txt  gaincalfast_20170921.104620.avg.txt  gaincalfast_20171126.141225_2.avg.txt
gaincalfast_20170505.163140.avg.txt  gaincalfast_20170613.184357.avg.txt  gaincalfast_20170731.122306.avg.txt  gaincalfast_20171009.151238.avg.txt  gaincalfast_20171126.141225.avg.txt
gaincalfast_20170515.170709.avg.txt  gaincalfast_20170619.104910.avg.txt  gaincalfast_20170810.170619.avg.txt  gaincalfast_20171026.164159.avg.txt)
for l in ${list2017[*]}
do
 ./GainCalFastFits_old /localdata/2017/CALIB/GainCal/$l
done

list2018=(gaincalfast_20180305.200728.avg.txt  gaincalfast_20180430.123258.avg.txt  gaincalfast_20180605.124858.avg.txt  gaincalfast_20180728.063248.avg.txt  gaincalfast_20180910.154327.avg.txt
gaincalfast_20180419.131317.avg.txt  gaincalfast_20180518.122939.avg.txt  gaincalfast_20180621.160623.avg.txt  gaincalfast_20180802.181425.avg.txt)
for l in ${list2018[*]}
do
 ./GainCalFastFits_old /localdata/2018/CALIB/GainCal/$l
done

#newfolder=FitResult0
#newfolder=FitResult0NDFM0Chi2m1000
#newfolder=FitResult0NDFM0Chi2m10
newfolder=FitResultErrorVCal80800_2
mkdir $newfolder
mv *root $newfolder  
mv *dat $newfolder
