#!/usr/bin/bash

#wget "https://127.0.0.1:80/control/record/stop?app=restream&name=dispenser128&rec=rec"
wget -O - "http://localhost:80/control/record/stop"
