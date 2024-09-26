#!/usr/bin/bash

echo "app ${1} name ${2}"

wget "https://127.0.0.1:80/control/record/start?app=restream&name=dispenser128&rec=rec"
