import sys
import os
import urllib2
import urlparse
import time


def downloadImages(url, out_path): 

    try:
        imgData = urllib2.urlopen(url).read()

        fileName = time.strftime('%Y_%m_%d_%H%M') + ".png"        
        output = open(os.path.join(out_path, fileName),'wb')
        output.write(imgData)
        output.close()

        print "Written ", fileName
        
    except Exception, e:
        print str(e)

# end of downloadImages


out_path = "scope_images"
image_url = "http://129.129.140.151:81/image.png"


# Make sure the output directory exists
try:
    os.mkdir(out_path)
except:
    pass

while True:
    
    downloadImages(image_url, out_path)        
    time.sleep(60. * 10) # Wait for 10 minutes
     

