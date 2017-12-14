#Project8: Just a quick script that will store the means with their errors for all the fills neing studied for every channel in order to be able to plot them as a function of the fills
#Beg. date: 18/10/2017

import matplotlib.pyplot as plt

fills = [6016, 6104, 6156, 6161, 6182, 6194, 6271, 6285]  #fill 6016 = vdM scan; fill 6194 = high PU

ch1 = [1.094, 1.108, 1.103, 1.099, 1.098, 1.115, 1.114, 1.121]
ch1_err = [0.149, 0.182, 0.194, 0.065, 0.050, 0.116, 0.089, 0.108]

ch2 = [1.003, 1.006, 1.008, 1.007, 1.005, 1.011, 1.015, 1.026]
ch2_err = [0.145, 0.035, 0.152, 0.027, 0.092, 0.054, 0.024, 0.059]

ch3 = [0.919, 0.790, 0.785, 0.794, 0.802, 0.783, 0.775, 0.765]
ch3_err = [0.134, 0.060, 0.171, 0.143, 0.101, 0.058, 0.046, 0.057]

ch5 = [0.910, 0.961, 0.968, 0.948, 0.954, 0.961, 0.957, 0.966]
ch5_err = [0.146, 0.101, 0.273, 0.041, 0.109, 0.117, 0.073, 0.108]

ch6 = [0.971, 1.040, 1.053, 1.032, 1.029, 1.043, 1.030, 1.035]
ch6_err = [0.155, 0.197, 0.243, 0.071, 0.121, 0.084, 0.107, 0.124]

ch7 = [1.076, 1.074, 1.083, 1.077, 1.083, 1.085, 1.085, 1.095] 
ch7_err = [0.150, 0.036, 0.237, 0.044, 0.049, 0.058, 0.022, 0.061]

ch8 = [0.926, 0.916, 0.916, 0.924, 0.919, 0.915, 0.925, 0.928]
ch8_err = [0.136, 0.118, 0.233, 0.121, 0.068, 0.064, 0.055, 0.069]

ch9 = [1.028, 1.017, 1.007, 1.023, 1.021, 1.004, 1.012, 1.016]
ch9_err = [0.144, 0.040, 0.165, 0.028, 0.036, 0.061, 0.048, 0.085]

ch10 = [1.136, 1.117, 1.106, 1.117, 1.117, 1.099, 1.105, 1.110]
ch10_err = [0.151, 0.108, 0.175, 0.055, 0.094, 0.058, 0.053, 0.063]

ch11 = [1.161, 1.162, 1.150, 1.167, 1.164, 1.168, 1.177, 1.197]
ch11_err = [0.151, 0.061, 0.206, 0.043, 0.075, 0.066, 0.042, 0.115]

ch12 = [1.087, 1.072, 1.066, 1.072, 1.078, 1.074, 1.077, 1.054]
ch12_err = [0.149, 0.094, 0.147, 0.115, 0.093, 0.057, 0.047, 0.105]

ch13 = [0.961, 0.995, 1.001, 0.988, 0.986, 1.001, 0.993, 0.984]
ch13_err = [0.153, 0.125, 0.180, 0.031, 0.077, 0.068, 0.040, 0.118]

ch14 = [0.872, 0.893, 0.897, 0.896, 0.892, 0.893, 0.889, 0.874]
ch14_err = [0.135, 0.052, 0.194, 0.090, 0.028, 0.053, 0.027, 0.106]

ch15 = [0.857, 0.848, 0.857, 0.856, 0.853, 0.848, 0.847, 0.828]
ch15_err = [0.138, 0.057, 0.205, 0.045, 0.112, 0.052, 0.042, 0.120]


#Plotting section
plt.errorbar(fills, ch1, yerr=ch1_err, linestyle="None", fmt="bo", ecolor="r", label ="norm_rates_avg")
plt.title("Mean with error for channel 1")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch2, yerr=ch2_err, linestyle="None", fmt="bo", ecolor="r")
plt.title("Mean with error for channel 2")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch3, yerr=ch3_err, linestyle="None", fmt="bo", ecolor="r")
plt.title("Mean with error for channel 3")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch5, yerr=ch5_err, linestyle="None", fmt="bo", ecolor="r")
plt.title("Mean with error for channel 5")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch6, yerr=ch6_err, linestyle="None", fmt="bo", ecolor="r")
plt.title("Mean with error for channel 6")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch7, yerr=ch7_err, linestyle="None", fmt="bo", ecolor="r")
plt.title("Mean with error for channel 7")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch8, yerr=ch8_err, linestyle="None", fmt="bo", ecolor="r", label ="norm_rates_avg")
plt.title("Mean with error for channel 8")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch9, yerr=ch9_err, linestyle="None", fmt="bo", ecolor="r", label ="norm_rates_avg")
plt.title("Mean with error for channel 9")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch10, yerr=ch10_err, linestyle="None", fmt="bo", ecolor="r", label ="norm_rates_avg")
plt.title("Mean with error for channel 10")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch11, yerr=ch11_err, linestyle="None", fmt="bo", ecolor="r", label ="norm_rates_avg")
plt.title("Mean with error for channel 11")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch12, yerr=ch12_err, linestyle="None", fmt="bo", ecolor="r", label ="norm_rates_avg")
plt.title("Mean with error for channel 12")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch13, yerr=ch13_err, linestyle="None", fmt="bo", ecolor="r", label ="norm_rates_avg")
plt.title("Mean with error for channel 13")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch14, yerr=ch14_err, linestyle="None", fmt="bo", ecolor="r", label ="norm_rates_avg")
plt.title("Mean with error for channel 14")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()


plt.errorbar(fills, ch15, yerr=ch15_err, linestyle="None", fmt="bo", ecolor="r", label ="norm_rates_avg")
plt.title("Mean with error for channel 15")
plt.xlabel("Fill")
plt.ylabel("Normalized average rate")
plt.plot((6000, 6300),(1,1), "g", label ="y=1")
#plt.legend(loc="best", fancybox=True, shadow=True)
plt.grid(True)
plt.show()
