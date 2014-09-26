#!/usr/bin/env python

"""
Download screenshot from the Oscilloscope downstairs every ten minutes.
(Used for the September 2014 PSI Diamond Testbeam)\
"""

###############################
# Imports
###############################

import os
import urllib2
import time


###############################
# Helper:
# downloadImage
###############################

def downloadImage(url, out_path):
    """Function to download image from an url and save it as timestamp.png"""

    try:
        imgData = urllib2.urlopen(url).read()

        fileName = time.strftime('%Y_%m_%d_%H%M') + ".png"        
        output = open(os.path.join(out_path, fileName),'wb')
        output.write(imgData)
        output.close()

        print "Written ", fileName
        
    except Exception, e:
        print str(e)
# end of downloadImage


###############################
# Configuration
###############################

out_path = "scope_images"
image_url = "http://129.129.140.151:81/image.png"

# Make sure the output directory exists
try:
    os.mkdir(out_path)
except:
    pass


###############################
# Main loop
###############################

while True:    
    downloadImage(image_url, out_path)        
    time.sleep(60. * 10) # Wait for 10 minutes
# end of main loop     

