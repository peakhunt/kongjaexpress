#!/bin/bash

SLAVE_ADDR=1
INTERVAL=50
TIMEOUT=10.0
BAUD=9600

./modpoll -m rtu -a $SLAVE_ADDR -r 100 -c 64 -t 4 -l $INTERVAL -o $TIMEOUT -b $BAUD -d 8 -s 1 -p none /dev/ttyUSB1
