#!/usr/bin/env python3

### Konstantinos Damanakis August 2020
### This script reads the csv files produced by the ROCEfficiency.sh bash script and plots the per ROC or/and per channel efficiency over integrated luminosity. 


import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import glob

###Read csv files####

folder = '/scratch/damanakis/PLTOffline/Efficiency_csv/'
csv_dir = glob.glob(folder + '*.csv')
print(csv_dir)
df = [pd.read_csv(f) for f in csv_dir]


######Declaration#####

eff0=[]
eff1=[]
eff2=[]
err0=[]
err1=[]
err2=[]
telescope=[]
telescope_err=[]

channel = [2, 4, 5, 8, 10, 11, 13, 14, 16, 17, 19, 20]  #exclude 7, 22, 23

lumi = [0.2036, 0.333, 0.856, 2.175, 2.603, 3.390, 4.209, 4.211, 4.411, 8.765, 11.942, 16.230, 21.956, 23.209, 25.738, 27.196, 32.105, 34.496, 38.515, 44.346, 44.753, 45.272, 51.106, 55.672, 61.261, 65.238, 69.411, 73.527, 84.070, 88.273, 94.905]

lumi_hvchange = [34.496, 55.672, 61.261, 84.070] #list with lumi values for which the HV setpoint has been changed
hv_setpoint = ['150V', '200V', '300V', '400V'] # list with HV setpoints
text_pos = [26, 47, 65, 87] #unfortunately it has to be hardcoded

y_low = 0.7 #y-axis lower limit
y_up = 1.00 #y-axis upper limit


####Fill arrays with efficiency values####

for ch in channel:
  for i in df:
    a = i.loc[(i['Channel']==ch) & (i['ROC']==0)]
    eff0.append(a['Efficiency'].values[0])
    err0.append(a.iloc[:, 5].values[0]) #error
    b = i.loc[(i['Channel']==ch) & (i['ROC']==1)]
    eff1.append(b['Efficiency'].values[0])
    err1.append(a.iloc[:, 5].values[0]) #error
    c = i.loc[(i['Channel']==ch) & (i['ROC']==2)]
    eff2.append(c['Efficiency'].values[0])
    err2.append(a.iloc[:, 5].values[0]) #error

 ###Calculate the correlation coefficient between all pair of planes of the same channel 
  coef_01 = np.corrcoef(eff0, eff1)
  coef_02 = np.corrcoef(eff0, eff2)
  coef_12 = np.corrcoef(eff1, eff2)
   

 #### If you want to plot the pertelescope efficiency
  for j in range(len(eff0)):
     telescope.append((eff0[j]+eff1[j]+eff2[j])/3) # Telescope efficiency
     var = ((err0[j])**2 +(err1[j])**2 +(err2[j])**2+ 2*coef_01*(err0[j])*(err1[j])+ 2*coef_02*(err0[j])*(err2[j])+ 2*coef_12*(err1[j])*(err2[j]))/3
     telescope_err.append(var)
    
###Plot Efficiency over Lumi
  plt.figure()
  plt.errorbar(lumi, eff0, err0, fmt= 'o-', color='blue', label= 'ROC0')
  plt.errorbar(lumi, eff1, err1,  fmt= 'o-', color='red', label ='ROC1')
  plt.errorbar(lumi, eff2, err2,  fmt= 'o-', color='green', label='ROC2')
 
  for j in range (len(lumi_hvchange)):
      plt.vlines(lumi_hvchange[j], y_low, y_up, linestyles='dashed') #set the dashed lines which indicate a HV setpoint change
      plt.text(text_pos[j], max(eff0), hv_setpoint[j])

  plt.ylim(y_low, y_up)
  plt.xlabel('Cumulative Integrated Luminosity [fb$^{-1} $]', fontsize=12)
  plt.ylabel('Efficiency', fontsize = 12)
  plt.title('ROC Efficiency over Integrated Luminosity Channel {}'.format(ch), fontsize=12)
  plt.legend(loc=4)
      #plt.show()
  plt.savefig('Efficiency_Ch{}.png'.format(ch))
  eff0.clear()
  eff1.clear()
  eff2.clear()
  err0.clear()
  err1.clear()
  err2.clear()
  telescope.clear()
  telescope_err.clear()
