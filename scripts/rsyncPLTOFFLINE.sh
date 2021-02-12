#!/usr/bin/env bash

ERRFMT="$(tput setaf 01)%s$(tput sgr 0 0)"
INFFMT="$(tput setaf 02)%s$(tput sgr 0 0)"

updateTimestamps(){
    # Update pltTimestamps.csv which lists SLINK and WORKLOOP timestamps corresponding to every fill (fill|fillDeclared|fillEnd|slinkTS|workloopTS)
    printf "\n%s\n" "UPDATING TIMESTAMPS..."
    # ./cern-get-sso-cookie.sh
    # ./PLT-timestamps.sh
    ./pltTimestamps.py
    cp 'pltTimestamps.csv' '/localdata/pltTimestamps.csv'
}

sshfsMount(){
    # SSHFS allows you to mount a remote filesystem using SFTP. Most SSH servers support and enable this SFTP access by default, so SSHFS is very simple to use - there's nothing to do on the server-side. [https://github.com/libfuse/sshfs]
    local remoteMount="$1"
    local localMount="$2"
    mkdir -p "${localMount}"
    if [[ -n "$( find "${localMount}" -maxdepth 0 -type d -empty 2>/dev/null )" ]]; then
        # Attempt to mount ${localMount} via sshfs if it is an empty directory (which it is when it's unmounted)
            # The find command returns a string only if the 0th-level of the specified directory is empty [https://superuser.com/a/352387]
        printf "${INFFMT}\n" "Mounting (via SSHFS) "${remoteMount}" -> "${localMount}"..."
        sshfs -o ProxyCommand="ssh ${USER}@cmsusr nc %h %p" "${remoteMount}" "${localMount}"
    fi
}

sshfsUnMount(){
    local localMount="$1"
    fusermount -u "${localMount}"
    rmdir         "${localMount}"
}

rsyncTransfer(){
    # verbosely transfer data. note that the --checksum flag is more accurate but far slower
    local description="$1"
    local localDir="$2"
    shift; shift
    local remoteFiles="$@"
    rsyncFlags="--archive --verbose --human-readable --progress" # [https://explainshell.com/explain?cmd=rsync+--archive+--checksum+--verbose+--human-readable+--progress]
    if [[ -n "$( find ${remoteFiles} 2>/dev/null )" ]]; then
        # Proceed with rsync command only if find can see the remote files. Discard stderr so that find returns an empty string if it sees nothing.
        printf "${INFFMT}\n" "${description}"
        rsync ${rsyncFlags} ${remoteFiles} "${localDir}"
    else
        printf "${ERRFMT}\n" "Could not find remote files in ${remoteFiles}"
    fi
}

pltslinkTransfer(){
    local lMount="/localdata/SSHFS.PLTSLINK"
    sshfsMount "root@pltslink:/" "${lMount}/"
    rsyncTransfer "Transferring ${year} SLINK files..." "/localdata/${year}/SLINK/" "${lMount}/root/FEDStreamReader/Slink_${year}"*
}

pltvmeTransfer(){
    local lMount="/localdata/SSHFS.PLTVME"
    local brildata="${lMount}/cmsnfsbrildata/brildata"
    sshfsMount "${USER}@pltvme1:/" "${lMount}/"
    # sshfsMount "${USER}@vmepc-s2d16-06-01:/" "${lMount}/"
    rsyncTransfer "Transferring ${year} WORKLOOP LOG files..."          "/localdata/${year}/WORKLOOP/"   "${lMount}/localdata/workloop_logs/WorkloopLog_${year}"*
    rsyncTransfer "Transferring ${year} BEAMSTATUSMONITOR LOG files..." "/localdata/${year}/WORKLOOP/"   "${lMount}/localdata/beamstatus_logs"*
    rsyncTransfer "Transferring ${year} CALIBRATION LOG files..."       "/localdata/${year}/CALIB/Logs/" "${lMount}/scratch/calib/${year}"*
    rsyncTransfer "Transferring ${year} CONFIGURATION BACKUP files..."  "/localdata/${year}/CONF/"       "${lMount}/scratch/conf.bkp/${year}"*
    rsyncTransfer "Transferring ${year} WORKLOOP files..."   "/localdata/${year}/WORKLOOP/"          "${brildata}/plt/workloop_data/Data_*_${year}"*
    rsyncTransfer "Transferring ${year} ULTRABLACK plots..." "/localdata/${year}/CALIB/Ultrablacks/" "${brildata}/plt/plots/Ultrablacks/${year}"*
    rsyncTransfer "Transferring ${year} GAINCAL files..."    "/localdata/${year}/CALIB/GainCal/"     "${brildata}/plt/calib/GainCal/gaincalfast_${year}"*
    rsyncTransfer "Transferring ${year} LEVELSCALIB files"   "/localdata/${year}/CALIB/Levels/"      "${brildata}/plt/plots/Levels/${year}"*
}

brildevTransfer(){
    # note that brildata _should_ be mounted on the vme machine under /cmsnfsbrildata/brildata
    local lMount="/localdata/SSHFS.BRILDATA"
    sshfsMount "${USER}@brildev:/brildata/" "${lMount}/"
    rsyncTransfer "Transferring ${year} WORKLOOP files..."   "/localdata/${year}/WORKLOOP/"          "${lMount}/plt/workloop_data/Data_"*"${year}"*
    rsyncTransfer "Transferring ${year} ULTRABLACK plots..." "/localdata/${year}/CALIB/Ultrablacks/" "${lMount}/plt/plots/Ultrablacks/${year}"*
    rsyncTransfer "Transferring ${year} GAINCAL files..."    "/localdata/${year}/CALIB/GainCal/"     "${lMount}/plt/calib/GainCal/gaincalfast_${year}"*
    rsyncTransfer "Transferring ${year} LEVELSCALIB files"   "/localdata/${year}/CALIB/Levels/"      "${lMount}/plt/plots/Levels/${year}"*
}

main(){
    if   [[ "$( hostname )" = "scx5-c2f06-36" ]]; then
        # for year in 2012 2013 2015 2016 2017 2018; do
        for year in 2018; do
            mkdir -p /localdata/${year}/SLINK/
            mkdir -p /localdata/${year}/WORKLOOP/
            mkdir -p /localdata/${year}/CONF/
            mkdir -p /localdata/${year}/CALIB/{Logs,Ultrablacks,GainCal,Levels}/
            pltslinkTransfer
            pltvmeTransfer
            # brildevTransfer
        done
    else
        printf "\n${ERRFMT}\n" "PLEASE RUN FROM PLTOFFLINE [scx5-c2f06-36]"
    fi
}

main
