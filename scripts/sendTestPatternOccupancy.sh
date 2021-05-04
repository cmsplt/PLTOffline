#!/usr/bin/env bash

quadrant="$1"
mountDir="${HOME/user/work}/pltslink2"

unmountSSHFS(){
    fusermount -u "${mountDir}" 2>/dev/null
}

mountSSHFS(){
    mkdir -p "${mountDir}" && sshfs 'pltslink2:/root/FEDStreamReader/' "${mountDir}/"
}

processSlinkFiles(){
    unmountSSHFS
    mountSSHFS
    local outDir="${HOME/user/work}/PLTOffline/plots/${quadrant}"
    local slinkFiles=("${mountDir}/${quadrant}/$(date +%Y%m%d)/Slink"*)
    for file in "${slinkFiles[@]}"; do
        local ts="$(echo ${file} | grep --extended-regex --only-matching "[0-9]{8}.[0-9]{6}")"
        if [[ -z "$(find "${outDir}/${ts}" -iname "histo_occupancy.root" 2>/dev/null)" ]]; then
            mkdir -p "${outDir}/${ts}/"
            cd "${HOME/user/work}/PLTOffline/"
            echo "./OccupancyPlots" "${file}"
            ./OccupancyPlots "${file}"
            mv -v "${HOME/user/work}/PLTOffline/histo_occupancy.root" "${outDir}/${ts}/histo_occupancy.root"
            mv -v "${HOME/user/work}/PLTOffline/plots/Occupancy*" "${outDir}/${ts}"
        fi
    done
    unmountSSHFS
}

processSlinkFiles
