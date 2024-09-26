#!/usr/bin/bash

echo "sub ${1} body ${2} video ${3} output ${4} from ${5} to ${6}"

mpack -s "${1}" -d "${2}" "${3}" -o "${4}"
