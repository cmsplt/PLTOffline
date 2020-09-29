#!/usr/bin/env python3

import os, sys, subprocess, json, itertools
import csv

def queryBXInfo( fill ):
    # http://linux.web.cern.ch/linux/docs/cernssocookie.shtml
    cerngetssocookie = subprocess.run( 'cern-get-sso-cookie --krb --reprocess --url https://cmsoms.cern.ch --outfile cmsoms.cookie'.split(), check = True )
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/OMS
    URL = f'https://cmsoms.cern.ch/agg/api/v1/bunches?filter[bunches][fill_number][EQ]={fill}&page[limit]=3564'
    jsonData = json.loads( subprocess.check_output( f'curl --silent --cookie cmsoms.cookie --globoff {URL}'.split(), encoding = 'utf-8' ) )
    instLumi = 'pileup' # 'peak_lumi' is also available. See CMSOMS bunch_info: https://cmsoms.cern.ch/cms/fills/bunch_info?cms_fill={fill}
    bxInfo = { bx['id'].split('_')[1]: bx['attributes'][instLumi] for bx in jsonData['data'] }
        # create a dictionary with 'BCID' as keys and 'instLumi' as values
    os.remove( 'cmsoms.cookie' )
    return bxInfo

def groupTrains( bxInfo ):
    # Group trains and and return as a list of lists
    # https://stackoverflow.com/a/2154437/13019084
    collidingBX = sorted( [ int(bx) for bx in bxInfo.keys() if bxInfo[bx] > 1 ] )
        # Filter 'BCIDs' with 'instLumi' > 1 into a list and sort values
        # 'instLumi' threshold may need to be adjusted depending on the fill
        # See CMSOMS bunch_info: https://cmsoms.cern.ch/cms/fills/bunch_info?cms_fill={fill}
    trains = []
    for key, group in itertools.groupby( enumerate(collidingBX), indexMinusBX ):
        # 'enumerate(collidingBX)' creates a tuple object consisting of the indices and 'BCIDs' in 'collidingBX'
        # 'itertools.groupby()' creates a tuple object consisting of the 'key' (the "category" given by 'indexMinusBX()')
        # and a group of the elements in 'enumerate(collidingBX)' sharing that "key/category"
        # https://sharats.me/posts/python-itertools-groupby-callable/
        group = [ groupTuple[1] for groupTuple in group ] # 'groupTuple[1]' corresponds to BCIDs and 'groupTuple[0]' to the indices
        # print( key ) # 'key' corresponds to 'bcid - index' for each group (see indexMinusBX())
        trains.append( group )
    return trains

def indexMinusBX( enumTuple ):
    # Subtract BCID from its index in collidingBX. This will be a constant integer until the next non-consecutive BCID.
    index = enumTuple[0]
    bcid  = enumTuple[1]
    return bcid - index

def main():
    fill = sys.argv[1]
    bxInfo = queryBXInfo( fill )
    trains = groupTrains( bxInfo )
    f = open('non_coll.csv', 'w')
    w = csv.writer(f, delimiter='\n')
    w.writerow(train[-1]+1  for train in trains if len(train)>1)
    
    print( [ train[-1] + 1 for train in trains if len(train)>1 ] ) # print non-colliding BX ID after each train

if __name__ == "__main__":
    main()
