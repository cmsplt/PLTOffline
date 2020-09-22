#!/bin/bash

slink2015=(Slink_20150821.210006.dat Slink_20150911.162826.dat Slink_20150924.021319.dat Slink_20151006.184854.dat Slink_20151020.042109.dat Slink_20151028.012656.dat Slink_20151102.190512.dat)     

fill2015=(4246 4349 4410 4467 4518 4540 4569)

gaincalfile2015=(GainCalFits_20150811.120552.dat GainCalFits_20150811.120552.dat GainCalFits_20150923.225334.dat GainCalFits_20150923.225334.dat GainCalFits_20150923.225334.dat GainCalFits_20151029.220336.dat GainCalFits_20151029.220336.dat) 


for index in ${!slink2015[*]}; 
do
  ./PulseHeights_new /localdata/2015/SLINK/${slink2015[$index]} FitResultErrorVCal80800_2/${gaincalfile2015[$index]}  ${fill2015[$index]}

done




slink2016=(Slink_20160428.202744.dat Slink_20160514.014829.dat Slink_20160617.000444.dat Slink_20160710.122326.dat Slink_20160723.181333.dat Slink_20160803.183752.dat Slink_20160813.144232.dat Slink_20160817.071542.dat Slink_20160905.202327.dat Slink_20160927.094645.dat Slink_20161012.012620.dat Slink_20161026.085537.dat)

fill2016=(4879 4924 5024 5085 5111 5161 5198 5211 5279 5340 5401 5451)

gaincalfile2016=(GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat GainCalFits_20160819.113115.dat)       


for index in ${!slink2016[*]};
do
  ./PulseHeights_new /localdata/2016/SLINK/${slink2016[$index]} FitResultErrorVCal80800_2/${gaincalfile2016[$index]}  ${fill2016[$index]}               

done


slink2017=(Slink_20170528.202456.dat Slink_20170615.042127.dat Slink_20170715.005934.dat Slink_20170802.044416.dat Slink_20170817.214233.dat Slink_20170826.135556.dat Slink_20170903.033049.dat Slink_20170925.014723.dat Slink_20171008.012429.dat Slink_20171020.121022.dat Slink_20171027.043638.dat Slink_20171119.204551.dat)   

fill2017=(5722 5834 5950 6035 6097 6136 6161 6241 6283 6312 6337 6398) 

gaincalfile2017=(GainCalFits_20170518.143905.dat GainCalFits_20170613.184357.dat GainCalFits_20170707.214121.dat GainCalFits_20170731.122306.dat GainCalFits_20170731.122306.dat GainCalFits_20170731.122306.dat GainCalFits_20170731.122306.dat GainCalFits_20170921.104620.dat GainCalFits_20170921.104620.dat GainCalFits_20170921.104620.dat GainCalFits_20171026.164159.dat GainCalFits_20171026.164159.dat)     


for index in ${!slink2017[*]};
do
  ./PulseHeights_new /localdata/2017/SLINK/${slink2017[$index]} FitResultErrorVCal80800_2/${gaincalfile2017[$index]}  ${fill2017[$index]} 

done


slink2018=(Slink_20180421.020428.dat Slink_20180427.144047.dat Slink_20180507.211825.dat Slink_20180516.191522.dat Slink_20180606.102329.dat Slink_20180711.222635.dat Slink_20180720.154754.dat Slink_20180805.064714.dat Slink_20180818.135037.dat Slink_20180902.112319.dat Slink_20180929.183744.dat Slink_20181021.210944.dat)

fill2018=(6584 6617 6654 6693 6762 6912 6953 7024 7063 7118 7236 7328)

gaincalfile2018=(GainCalFits_20180419.131317.dat GainCalFits_20180419.131317.dat GainCalFits_20180419.131317.dat GainCalFits_20180419.131317.dat GainCalFits_20180605.124858.dat GainCalFits_20180621.160623.dat GainCalFits_20180621.160623.dat GainCalFits_20180621.160623.dat GainCalFits_20180621.160623.dat GainCalFits_20180621.160623.dat GainCalFits_20180621.160623.dat GainCalFits_20180621.160623.dat)         

for index in ${!slink2018[*]};
do
   ./PulseHeights_new /localdata/2018/SLINK/${slink2018[$index]} FitResultErrorVCal80800_2/${gaincalfile2018[$index]}  ${fill2018[$index]}  

done

                                                                                                           
