#!/bin/bash

# to do:
    # implement functionality for CMS machines:
        # pip install --no-index --find-links=file:///nfshome0/lumipro/installers/linux-64 --user brilws
        # [https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#Installation-of-brilws]

wDir="${HOME}/.local/venv/plt" # stuff will be installed here!

defineFunctions(){

    errFmt="$( tput setaf 01)%s$(tput sgr 0 0)"
    infFmt="$(tput setaf 02)%s$(tput sgr 0 0)"

    continuePrompt(){
        # Print positional parameter "$1" and prompt user for input before proceeding
        printf "\n${errFmt}\n" "${1}"
        printf "${infFmt}\n" "Press <ENTER> to continue..."
        read -s -p ''
    }

    checkEmptyDir(){
        # Check if positional parameter "$1" is an empty or non-existent directory [https://superuser.com/a/352387]
        find "${1}" -maxdepth 0 -type d -empty 2>&1
            # The find command will print "$1" if it is an empty directory
            # An error will be redirected to stdout if "$1" doesn't exist
    }

    configureBrilconda(){
        # Set up python3 from Brilconda3
        # [https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html]
        brilcondaPath '/nfshome0/lumipro/brilconda3/bin' || brilcondaPath '/cvmfs/cms-bril.cern.ch/brilconda3/bin' || installBrilcondaVenv
        printf "\n${infFmt}\n" "$( command -v python3 )"
    }

    brilcondaPath(){
        # Prepend "$HOME/.local/bin" and positional argument "$1" (if it an existent directory) to "$PATH"
        # [https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#Prerequisite-for-running-and-installing-brilws]
        local localBin="$HOME/.local/bin"
        [[ -d "${1}" ]] && export PATH="${localBin}:${1}:${PATH}"
    }

    installBrilcondaVenv(){
        # Install local brilconda virtual environment
        local hostRecomm="Running from a CERN host (with a /cvmfs mount) is highly recommended"
        local miniconda3="Otherwise, please confirm to install a local brilconda virtual environment to ${HOME}/brilconda3"
        local unameKernel="$( uname --kernel-name )"
        [[ "${unameKernel}" == "Linux"* ]]  && url="https://cern.ch/cmslumisw/installers/linux-64/Brilconda-3.2.16-Linux-x86_64.sh"
        [[ "${unameKernel}" == "Darwin"* ]] && url="https://cern.ch/cmslumisw/installers/macos-64/Brilconda-3.2.16-MacOSX-x86_64.sh"
        printf "${errFmt}\n${errFmt}\n" "${hostRecomm}" "${miniconda3}"
        read -p "(y/n) " -n 1
        [[ "${REPLY}" =~ ^[Yy]$ ]] && [[ -n "${url}" ]] && { wget "${url}" && bash "${url##*/}" -b -p "${HOME}/brilconda3" ; }
    }

    activateVenv(){
        # Activate the plt venv if it exists and, if it doesn't, prompt user to create & activate it
        local venvActivate="${wDir}/bin/activate"
        local venvInstructions="${wDir} virtual environment activated. Run 'deactivate' to exit the venv."
        local venvInfo="A new virtual environment will be created in ${wDir}"
        if [[ -f   "${venvActivate}" ]]; then
            source "${venvActivate}" && printf "\n${infFmt}\n" "${venvInstructions}"
        else
            continuePrompt "${venvInfo}"
            mkdir -vp  "${wDir%/*}" # create parent directory (path) to "wDir" if non-existent
            python3 -m venv --system-site-packages "${wDir}"
                # --system-site-packages will allow the venv to "inherit" the system environment (e.g. brilconda-installed packages)
                # [https://stackoverflow.com/a/19459977/13019084]
            source "${venvActivate}" && printf "\n${infFmt}\n" "${venvInstructions}"
        fi
        printf "\n${infFmt}\n" "$( command -v python3 )"
    }

    pipInstall(){
        # Install or upgrade package given by positional parameter "$1" (intended for use within a python3 venv)
        printf "\n${infFmt}\n" "python3 -m pip install --upgrade ${1}"
        python3 -m pip install --upgrade "${1}"
    }

    pipInstallPackages(){
        # Install/upgrade packages
        pipInstall pip
        pipInstall brilws # [https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html#Installation-of-brilws]
        # pipInstall pytimber # [https://github.com/rdemaria/pytimber]
        # pipInstall h5py
        # pipInstall matplotlib
        # pipInstall more-itertools
        # pipInstall numpy
        # pipInstall pandas
        # pipInstall pytz
        # pipInstall requests
        # pipInstall scipy
        # pipInstall tables
        # pipInstall tqdm
    }

    gitClone(){
        # Define variables used for testing github/gitlab SSH keys, git clone, etc
        gitServ="$1"
        gitUser="$2"
        gitRepo="$3"
        gitConfigGlobal
        [[ "${gitServ}" = "github" ]] && gitDomain="github.com"     && sett="settings" && port="22"
        [[ "${gitServ}" = "gitlab" ]] && gitDomain="gitlab.cern.ch" && sett="profile"  && port="7999"
        gitConfigKeysSSH
        local gitSSH="ssh://git@${gitDomain}:${port}/${gitUser}/${gitRepo}.git"
        # klist >/dev/null && gitKRB="https://:@gitlab.cern.ch:8443/${gitUser}/${gitRepo}.git"
        cd "${wDir}"
        if [[ "$( checkEmptyDir ${gitRepo} )" ]]; then
            printf "\n${infFmt}\n" "git clone ${gitSSH}"
            git clone "${gitSSH}"
        else
            cd "${wDir}/${gitRepo}"
            printf "\n${infFmt}\n" "cd ${wDir}/${gitRepo}; git pull"
            git pull
        fi
        gitInstallCompile
        cd "${wDir}"
    }

    gitInstallCompile(){
        cd "${wDir}/${gitRepo}"
        printf "\n${infFmt}\n" "cd ${wDir}/${gitRepo}; make/scons/etc"
        [[ -f "requirements.txt" ]] && python3 -m pip install -r requirements.txt
        [[ -f "setup.py" ]] && python3 setup.py install
        [[ "${gitRepo}" = "cmsplt" ]] && cd "interface/" && scons
        #local root="/cvmfs/cms.cern.ch/slc5_amd64_gcc481/cms/cmssw/CMSSW_7_1_1/external/slc5_amd64_gcc481/bin/thisroot.sh"
        local root="/cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.06.08/x86_64-centos7-gcc48-opt/root/bin/thisroot.sh"
        [[ "${gitRepo}" = "PLTOffline" ]] && source "${root}" && make # try to restore "$PATH" from "PATHbkp=$PATH"?
        cd "${wDir}"
    }

    gitConfigGlobal(){
        # Prompt user to set up git global config, if needed
        # [https://git-scm.com/book/en/v2/Getting-Started-First-Time-Git-Setup]
        if [[ ! -f "${HOME}/.gitconfig" ]] && [[ ! -f "${HOME}/.config/git/config" ]]; then
            printf "\n${errFmt}\n${errFmt}\n${errFmt}\n"                            \
                "Please create a GitHub account, if needed:"                        \
                "[http://cms-sw.github.io/faq.html#how-do-i-subscribe-to-github]"   \
                "And provide your fullname and then ${gitServ} email & username:"
        fi
        gitConfigGlobalEntry "user.name"
        gitConfigGlobalEntry "user.email"
        gitConfigGlobalEntry "user.${gitServ}" # Required by 'git cms-init'
            # [https://github.com/cms-sw/cms-git-tools/blob/master/git-cms-init#L151]
        # git config --global http.emptyAuth  true
            # [https://docs.gitlab.com/ee/integration/kerberos.html#http-basic-access-denied-when-cloning]
        # git config --global push.default simple
            # [https://git-scm.com/docs/git-config.html#git-config-pushdefault]
    }

    gitConfigGlobalEntry(){
        # If "$configKey" is missing from git config, prompt user and set it (in a sub-shell)
        local configKey="$1"
        local configValue
        gitConfigGet="$( git config --get "${configKey}" )"
        promptKey="Please provide ${gitServ} ${configKey}"
        if [[ -z "${gitConfigGet}" ]]; then
            printf "\n${errFmt}\n" "${promptKey}"
            read -p "${configKey}: " configValue
            git config --global "${configKey}" "${configValue}"
        fi
    }

    gitConfigKeysSSH(){
        # Test SSH key authentication for github/gitlab and guide SSH key set up if it fails; then retest
        gitTestSSH="ssh -T -p ${port} git@${gitDomain}"
        exitCodeGitTestSSH="$( ${gitTestSSH} 2>&1 | grep -qiE 'successfully|welcome'; echo $? )"
        if [[ "${exitCodeGitTestSSH}" != 0 ]]; then
            keyGenSSH && sshConfigAppend
        fi
    }

    keyGenSSH(){
        # Create a new ssh key (with empty passphrase) and print instructions
        gitKeysURL="https://${gitDomain}/${sett}/keys"
        continuePrompt "${gitServ} SSH key authentication failed. A new ssh key will be generated at ${HOME}/.ssh/id_rsa_${gitServ}"
        ssh-keygen -o -t rsa -b 4096 -N '' -C "$( git config --get user.email )" -f "${HOME}/.ssh/id_rsa_${gitServ}"
            # [https://help.github.com/en/github/authenticating-to-github/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent]
            # [https://docs.gitlab.com/ee/ssh/#rsa-ssh-keys]
            # -o Causes ssh-keygen to save private keys using the new OpenSSH format rather than the more compatible PEM format.
                # [https://docs.gitlab.com/ee/ssh/README.html#rsa-keys-and-openssh-from-versions-65-to-78]
            # -t Specifies the type of key to create.
            # -b Specifies the number of bits in the key to create. For RSA keys, the minimum size is 1024 bits and the default is 2048 bits.
            # -N Provides the new passphrase ('' sets an empty passphrase)
            # -C Provides a new comment
            # -f Specifies the filename of the key file
        printf "\n${errFmt}\n\n" "Navigate to [${gitKeysURL}], create a new SSH key, and paste the following text under the 'Key' textbox ('Title' can be 'plt', for example):"
        cat "${HOME}/.ssh/id_rsa_${gitServ}.pub"
    }

    sshConfigAppend(){
        # Append a corresponding entry to ~/.ssh/config
        continuePrompt "A 'Host ${gitDomain}' entry will be appended to ${HOME}/.ssh/config"
        printf "\n%s\n\t%s\n\t%s\n\t%s"                             \
            "Host ${gitDomain}"                                     \
            "IdentityFile                ~/.ssh/id_rsa_${gitServ}"  \
            "StrictHostKeyChecking       no"                        \
            "UserKnownHostsFile          /dev/null"                 >> "${HOME}/.ssh/config"
    }

}

main(){
    configureBrilconda
    activateVenv
    if [[ "${VIRTUAL_ENV##*/}" = "${wDir##*/}" ]]; then
        pipInstallPackages
        # gitClone "gitlab" "cmsoms" "oms-api-client"
        gitClone "github" "cmsplt" "PLTOffline"
        # gitClone "gitlab" "bril"   "cmsplt"
    fi
}

if [[ "$0" = "$BASH_SOURCE" ]]; then
    # [https://stackoverflow.com/a/59274815/13019084]
    printf "\n%s\n%s\n" "Please 'source' the script instead of executing it:" "source ${0/.\//}"
else
    # Run main function in a sub-shell when the script is 'sourced'
    # so that all functions and variables are local and it can 'exit' without logging out the ssh session
    ( defineFunctions; main )
    source "${wDir}/bin/activate" # run activateVenv to load the venv after exiting the script
    unset wDir
fi
