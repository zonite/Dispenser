#!/usr/bin/bash

#apt-get install mpack


#mpack -s "testi" -d /tmp/body /tmp/test.flv zonite@nykyri.eu

#echo "from ${1} to ${2} message ${3}"

#mpack -s "${1}" -d "${2}" "${3}" -o "${4}"

sendmail -f "${1}" "${2} < "${3}"
