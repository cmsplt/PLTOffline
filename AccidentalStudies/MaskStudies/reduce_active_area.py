#!/usr/bin/env python3

# This script takes a mask file and reduces the active area so it can be used for testing
# different mask sizes. Note that it makes some pretty specific assumptions about the format,
# but since the masks are usually all generated with the same format this shouldn't be
# a problem. It was designed for the mask used at the beginning of 2016 operations (when this
# study was first carried out), Mask_May2016_v1.txt.
#
# Usage: reduce_active_area.py [mask file] [center columns] [center rows] [outer columns] [outer rows]

ncols = 52
nrows = 80

import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("mask_file", help="Name of input mask file")
parser.add_argument("center_cols", type=int, help="Number of columns in active area of central plane")
parser.add_argument("center_rows", type=int, help="Number of rows in active area of central plane")
parser.add_argument("outer_cols", type=int, help="Number of columns in active area of outer planes")
parser.add_argument("outer_rows", type=int, help="Number of rows in active area of outer planes")
args = parser.parse_args()

cur_id = [-1, -1, -1, -1]   # mFEC, mFECCh, hub, roc
cur_edges = [-1, -1, -1, -1] # left, right, bottom, top
cur_nlines = 0

with open(args.mask_file) as infile:
    for line in infile:
        # Skip empty or comment lines
        if len(line) == 0 or line[0] == '#':
            continue

        # If this doesn't contain a range, then it's probably just a mask for a dead pixel.
        # We can just ignore these for this purpose.
        if line.find('-') == -1:
            continue

        fields = line.rstrip().split(" ")

        # Consistency checks
        if cur_nlines != 0 and cur_id != fields[0:4] :
            print("Warning: ID fields",fields[0:4],"didn't match expected", cur_id)
        if fields[6] != '0':
            print("Warning: pixel status is unexpected value of", fields[6])

        cur_id = fields[0:4]

        if fields[5] == '0-'+str(nrows-1):
            these_cols = [int(x) for x in fields[4].split('-')]
            if these_cols[0] == 0:
                # left edge
                cur_edges[0] = these_cols[1]+1
            elif these_cols[1] == ncols-1:
                # right edge
                cur_edges[1] = these_cols[0]-1
            else:
                # huh?
                print("Warning: couldn't understand line", line)

        elif fields[4] == '0-'+str(ncols-1):
            these_rows = [int(x) for x in fields[5].split('-')]
            if these_rows[0] == 0:
                # bottom edge
                cur_edges[2] = these_rows[1]+1
            elif these_rows[1] == nrows-1:
                # top edge
                cur_edges[3] = these_rows[0]-1
            else:
                # huh?
                print("Warning: couldn't understand line", line)

        else:
            # also confused in this case
            print("Warning: couldn't understand line", line)

        cur_nlines += 1
        
        # If we've read in all four lines for a given ROC then do the resizing!
        if cur_nlines == 4:
            cur_cols = cur_edges[1]-cur_edges[0]+1
            cur_rows = cur_edges[3]-cur_edges[2]+1
            if cur_id[3] == '1':
                target_cols = args.center_cols
                target_rows = args.center_rows
            else:
                target_cols = args.outer_cols
                target_rows = args.outer_rows

            # Figure out how much to shrink.
            new_edges = list(cur_edges)
            if (cur_cols - target_cols) % 2 == 0:
                new_edges[0] += (cur_cols - target_cols)//2
                new_edges[1] -= (cur_cols - target_cols)//2
            else:
                # Rather arbitrarily take away the extra column on the right side
                new_edges[0] += (cur_cols - target_cols)//2
                new_edges[1] -= (cur_cols - target_cols)//2 + 1

            if (cur_rows - target_rows) % 2 == 0:
                new_edges[2] += (cur_rows - target_rows)//2
                new_edges[3] -= (cur_rows - target_rows)//2
            else:
                new_edges[2] += (cur_rows - target_rows)//2
                new_edges[3] -= (cur_rows - target_rows)//2 + 1

            #print("Active area for",cur_id,"was",cur_edges,"->",new_edges)
        
            # Now, print it all out.
            print(" ".join(cur_id), "0-"+str(new_edges[0]-1), "0-79 0")
            print(" ".join(cur_id), str(new_edges[1]+1)+"-51 0-79 0")
            print(" ".join(cur_id), "0-51 0-"+str(new_edges[2]-1), "0")
            print(" ".join(cur_id), "0-51", str(new_edges[3]+1)+"-79 0")

            # and reset
            cur_id = [-1, -1, -1, -1]   # mFEC, mFECCh, hub, roc
            cur_edges = [-1, -1, -1, -1] # left, right, top, bottom
            cur_nlines = 0
