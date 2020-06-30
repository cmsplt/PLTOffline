#!/bin/bash

# CERN Single Sign on authentication using scripts/programs [http://linux.web.cern.ch/linux/docs/cernssocookie.shtml]
    # cern-get-sso-cookie acquires CERN Single Sign-On cookie using Kerberos credentials or user certificate and stores it in a file for later usage with tools like curl, wget or others

cert="${1:-${HOME}/private/${USER}.p12}"
PEM="${cert/.p12/.pem}"
KEY="${cert/.p12/.key}"
TMP="${cert/.p12/.tmp}"

errFmt="$( tput setaf 01)%s$(tput sgr 0 0)"
infoFmt="$(tput setaf 02)%s$(tput sgr 0 0)"

convertCertificateFiles(){
    # Certificate-based authentication allows unattended authentication (until the certificate expires).
    printf "${errFmt}\n"  "${cert} user certificate/key file must be converted to specific formats"
    printf "${infoFmt}\n"  "The user certificate needs to be passwordless, so import password should be left blank"
    openssl pkcs12 -clcerts -nokeys -in "${cert}" -out "${PEM}"
    printf "${infoFmt}\n"  "Import password should be blank again. A PEM pass phrase is required (but only needed in the next step)"
    openssl pkcs12 -nocerts         -in "${cert}" -out "${TMP}"
    printf "${infoFmt}\n"  "Please repeat the pass phrase once more"
    openssl rsa                     -in "${TMP}"  -out "${KEY}"
    rm    -f  "${TMP}"
    chmod 644 "${PEM}"
    chmod 400 "${KEY}"
}

checkUserCert(){
    local certInstr="Please specify path to user certificate (.p12) file. If needed, first acquire one from the CERN Certification Authority: https://ca.cern.ch/ca/user/Request.aspx?template=EE2User"
    if [[ ! -f "${cert}" ]]; then
        printf "${errFmt}\n" "${certInstr}"
        return 1
    elif [[ ! -f "${PEM}" ]] || [[ ! -f "${KEY}" ]]; then
        convertCertificateFiles && return 0 || return 1
    fi
}

authenticate(){
    local service="$1"
    local method="$2"
    local URL="https://${service}.cern.ch/"
    local outFile="${HOME}/private/${service}.cookie"
    local success="${service} authentication successful: ${outFile}"
    local failure="${service} authentication unsuccessful :("
    [[ "${method}" = "cert" ]] && {
        cern-get-sso-cookie --verbose --reprocess --cert ${PEM} --key "${KEY}" --url "${URL}" --outfile "${outFile}" && printf "${infoFmt}\n" "${success}" || printf "${errFmt}\n" "${failure}";
    }
    [[ "${method}" = "krb"  ]] && {
        cern-get-sso-cookie --verbose --reprocess --krb --url "${URL}" --outfile "${outFile}" && printf "${infoFmt}\n" "${success}" || printf "${errFmt}\n" "${failure}";
    }
}

checkUserCert && authenticate "cmsoms" "cert"
# checkUserCert && authenticate "cmswbm" "cert"
# authenticate "cmsoms" "krb"
# authenticate "cmswbm" "krb"

unset cert PEM KEY TMP
