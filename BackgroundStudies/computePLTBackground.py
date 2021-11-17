#!/nfshome0/lumipro/brilconda/bin/python

# This script extracts the PLT background values from an HD5 file so they can be compared to the BCM1F values.
# No calibration is applied to the PLT background values, other than the normalization to the beam intensities.
#
# Warning: this script was designed for a 2016 fill (fill 5005), when the /pltaggzero table actually contained
# the raw number of triple coincidences, rather than the raw number of zeros. As a consequence I have to
# assume that all rows contain 4096 orbits. If you're working on more recent data, you'll need to change the
# calculation of f_0 from (4096-data)/4096 to data/4096, and possibly also check to make sure that there are
# actually 4096 orbits in each row (which is easily done by taking the closest multiple of 1024 to the maximum
# number of zeroes over all bunches).
#
# Basic principles: If we denote the collision time as t=0, then because of the position of the PLT, the
# incoming beam (beam 1 on the +z end and beam 2 on the -z end) arrives at t=-6 ns, while the outgoing beam
# (beam 1 on the -z end and beam 2 on the +z end) arrives at t=+6 ns. In the case of collisions, t=+6 ns is
# also when the collision products arrive (at both ends of the PLT), so any beam-induced background signal
# from the outgoing beam will be unmeasurable against the much larger collision signal.
# Thus, there are two basic possibilities:
# 1) method A: look for BXs where only one beam is present, so no collisions (noncolliding bunches), and use
# the outgoing beam signal to measure the beam background (-z measures beam 1, and +z measures beam 2)
# 2) method B: because of the 12 ns difference, the incoming beam signal may appear in the previous BX,
# so use the incoming beam signal in bunches preceding colliding bunches (but with no collisions themselves)
# to measure the beam background (-z measures beam 2, and +z measures beam 1)
# Note that noncolliding bunches can be used for both method A in their BX, and method B in the preceding BX.

import tables
import math

# We're only interested in the last two runs of the fill, since everything before then is before the test was
# done.
input_files = [
"/brildata/16/5005/5005_274958_1606111202_1606111411.hd5",
"/brildata/16/5005/5005_274959_1606111414_1606111641.hd5"]
output_file_bcm1f = "background_bcm1f.csv"
output_file_plt = "background_pltz.csv"
nbx = 3564

for (i_file, input_file) in enumerate(input_files):
    print "Beginning processing of", input_file
    h5file = tables.open_file(input_file)

    # Originally this was set up to populate everything in overall_results so that it could all be written out
    # at the end, but then it was easier just to write as we went, so we don't really need this any more. I've
    # left it commented out just in case it does become useful in the future.
    # overall_results = {}

    # First, get the bunch configuration. I assume it doesn't actually change during the fill, so we can just
    # take the first row.

    beam_info = h5file.get_node("/beam")[0]

    noncoll_bunches_1 = []
    noncoll_bunches_2 = []
    precoll_bunches_1 = []
    precoll_bunches_2 = []

    # Go through the bunches looking for:
    # - bunches filled in one beam and not the other
    # - bunches empty in one beam and filled in the next BX in the other beam
    # Warning: all bunch numbers are 0-indexed

    for ibx in range(nbx):
        if beam_info['bxconfig1'][ibx] == 1 and beam_info['bxconfig2'][ibx] == 0:
            noncoll_bunches_1.append(ibx)
        if beam_info['bxconfig1'][ibx] == 0 and beam_info['bxconfig2'][ibx] == 1:
            noncoll_bunches_2.append(ibx)
        if (ibx != nbx-1 and beam_info['bxconfig2'][ibx] == 0 and beam_info['bxconfig1'][ibx+1] == 1):
            precoll_bunches_1.append(ibx)
        if (ibx != nbx-1 and beam_info['bxconfig1'][ibx] == 0 and beam_info['bxconfig2'][ibx+1] == 1):
            precoll_bunches_2.append(ibx)

    print noncoll_bunches_1
    print noncoll_bunches_2
    print precoll_bunches_1
    print precoll_bunches_2

    # Next, get the beam intensity and store it in an array so we can use it to normalize the PLT background.
    # Note that since the time granularity of the beam data is not necessarily the same as from the lumi
    # detectors, we have to store it separately. However we do assume that it's in proper order.
    beam_data = []
    for row in h5file.get_node("/beam").iterrows():
        run = row['runnum']
        ls = row['lsnum']
        nb = row['nbnum']
        time_sec = row['timestampsec']
        time_msec = row['timestampmsec']
        beam_data.append({'s': time_sec, 'ms': time_msec, 'beam1': row['bxintensity1'], 'beam2': row['bxintensity2']})

    # Next, get the BCM1F data and store it in our results, and write it out to a file.
    if i_file == 0:
        bcm1f_out = open(output_file_bcm1f, "w")
        bcm1f_out.write("#time_sec,time_ms,run,ls,nb,bcm1f_minus,bcm1f_plus\n")
    else:
        bcm1f_out = open(output_file_bcm1f, "a")
    for row in h5file.get_node("/bcm1fbkg").iterrows():
        run = row['runnum']
        ls = row['lsnum']
        nb = row['nbnum']
        # if run not in overall_results:
        #     overall_results[run] = {}
        # if ls not in overall_results[run]:
        #     overall_results[run][ls] = {}
        # if nb not in overall_results[run][ls]:
        #     overall_results[run][ls][nb] = {}
        # overall_results[run][ls][nb]["bcm1fplus"] = row['plusz']
        # overall_results[run][ls][nb]["bcm1fminus"] = row['minusz']
        output_data = [row['timestampsec'], row['timestampmsec'], run, ls, nb, row['minusz'], row['plusz']]
        bcm1f_out.write(",".join([str(x) for x in output_data]) + "\n")
    bcm1f_out.close()

    # Now, get the per-bunch PLT data. Unfortunately, since we need the per-channel data, we have to go back to
    # the raw counts in /pltaggzero.

    last_chid = -1
    nrows = -1
    cur_run = -1
    cur_ls = -1
    cur_nb = -1
    cur_sec = -1
    cur_ms = -1

    totrows = len(h5file.get_node("/pltaggzero"))

    if (i_file == 0):
        plt_out = open(output_file_plt, "w")
        plt_out.write("#time_sec,time_ms,run,ls,nb,plt_bkg_A1,plt_bkg_A2,plt_bkg_B1,plt_bkg_B2\n")
    else:
        plt_out = open(output_file_plt, "a")

    last_computed_run = -1
    last_computed_ls = -1
    last_computed_nb = -1

    # A subroutine to compute and write out the PLT background. This is here because we need to call it twice,
    # once when we're looping and change to a new nibble, and once when we're done to get the last nibble.

    def compute_background(time_sec, time_ms, bxdata_minus, n_chan_minus, bxdata_plus, n_chan_plus, bxdata_tot, n_chan_tot):
        global last_computed_run
        global last_computed_ls
        global last_computed_nb
        # Find the corresponding entry in the beam data. This should be the latest entry before the time that we're looking for.
        beam_i = -1
        for i in range(len(beam_data)):
            if beam_data[i]['s'] > time_sec or (beam_data[i]['s'] == time_sec and beam_data[i]['ms'] > time_msec):
                if (i == 0):
                    print "Warning: no beam data before requested time in", cur_run, cur_ls, cur_nb
                    beam_i = 0
                else:
                    beam_i = i-1
                break

        if beam_i == -1:
            print "Warning: no beam data after requested time in", cur_run, cur_ls, cur_nb
            beam_i = len(beam_data)-1

        # There are two ways to apply the normalization by beam intensity, either normalizing per-bunch and
        # then averaging the results, or summing per-bunch and dividing by the total beam current for those
        # bunches. I'm not sure which is inherently better, but as the BCM1F processor does the latter, I'll
        # follow the same convention.
        bkgndA1 = 0
        bkgndA2 = 0
        bkgndB1 = 0
        bkgndB2 = 0
        bcA1 = 0
        bcA2 = 0
        bcB1 = 0
        bcB2 = 0
        for ibx in noncoll_bunches_1:
            bkgndA1 += bxdata_minus[ibx]
            bcA1 += beam_data[beam_i]['beam1'][ibx]
        for ibx in noncoll_bunches_2:
            bkgndA2 += bxdata_plus[ibx]
            bcA2 += beam_data[beam_i]['beam2'][ibx]
        # For precolliding bunches we want to normalize by the intensity of the NEXT bunch,
        # since that's what we are actually expecting the background to be from
        for ibx in precoll_bunches_1:
            bkgndB1 += bxdata_plus[ibx]
            bcB1 += beam_data[beam_i]['beam1'][ibx+1]
        for ibx in precoll_bunches_2:
            bkgndB2 += bxdata_minus[ibx]
            bcB2 += beam_data[beam_i]['beam2'][ibx+1]

        # Average over channels and divide by beam intensity.
        bkgndA1 /= n_chan_minus*bcA1
        bkgndA2 /= n_chan_plus*bcA2
        bkgndB1 /= n_chan_minus*bcB1
        bkgndB2 /= n_chan_minus*bcB2

        # overall_results[cur_run][cur_ls][cur_nb]["pltbkgndA1"] = bkgndA1
        # overall_results[cur_run][cur_ls][cur_nb]["pltbkgndA2"] = bkgndA2
        # overall_results[cur_run][cur_ls][cur_nb]["pltbkgndB1"] = bkgndB1
        # overall_results[cur_run][cur_ls][cur_nb]["pltbkgndB2"] = bkgndB2

        last_computed_run = cur_run
        last_computed_ls = cur_ls
        last_computed_nb = cur_nb

        output_data = [time_sec, time_ms, cur_run, cur_ls, cur_nb, bkgndA1, bkgndA2, bkgndB1, bkgndB2]
        plt_out.write(",".join([str(x) for x in output_data]) + "\n")

        return

    for row in h5file.get_node("/pltaggzero").iterrows():
        # Unfortunately the hd5 files for these runs contain some duplicated data. I have no idea how this
        # happened but here we are! So basically if anything is less than or equal to a nibble we've already done,
        # then reject it.
        if row['lsnum'] < last_computed_ls or (row['lsnum'] == last_computed_ls and row['nbnum'] <= last_computed_nb):
            print "Warning: duplicated data in ls/nb/ch",row['lsnum'],row['nbnum'],row['channelid']
            continue

        # If the channel number has gone down, then we must be starting a new nibble, so calculate the results
        # from the last nibble and store them.
        if row['channelid'] < last_chid:
            compute_background(cur_sec, cur_ms, bxdata_minus, n_chan_minus, bxdata_plus, n_chan_plus, bxdata_tot, n_chan_tot)

        # For the first row, or if we're starting a new nibble, store the information and reset the counters.
        if nrows == -1 or row['channelid'] < last_chid:
            cur_run = row['runnum']
            cur_ls = row['lsnum']
            cur_nb = row['nbnum']
            cur_sec = row['timestampsec']
            cur_ms = row['timestampmsec']
            n_chan_minus = 0
            n_chan_plus = 0
            n_chan_tot = 0
            bxdata_minus = [0]*nbx
            bxdata_plus = [0]*nbx
            bxdata_tot = [0]*nbx
            nrows += 1
            if (nrows % 10) == 0:
                print "Processed",nrows,"rows"

        # Sanity check
        if cur_run != row['runnum'] or cur_ls != row['lsnum'] or cur_nb != row['nbnum']:
            print "Warning: inconsistent run, LS, or nibble number (expected", cur_run, cur_ls, cur_nb, "got",
            row['runnum'], row['lsnum'], row['nbnum'],")"
        last_chid = row['channelid']

        #print cur_run, cur_ls, cur_nb, last_chid

        n_chan_tot += 1
        if row['channelid'] < 8:
            n_chan_minus += 1
        else:
            n_chan_plus += 1

        for ibx in range(nbx):
            mu = -math.log((4096.0-row['data'][ibx])/4096.0)
            if row['channelid'] < 8:
                bxdata_minus[ibx] += mu
            else:
                bxdata_plus[ibx] += mu
            bxdata_tot[ibx] += mu

    # Finally, when we're done looping, call compute_background() one more time so the last nibble gets stored
    compute_background(cur_sec, cur_ms, bxdata_minus, n_chan_minus, bxdata_plus, n_chan_plus, bxdata_tot, n_chan_tot)

    plt_out.close()
    h5file.close()
