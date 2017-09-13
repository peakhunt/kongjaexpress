#!/bin/bash

IP=192.168.1.100
SLAVE_ADDR=2
TIMEOUT=10.0
INTERVAL=1000
PORT=12345

./modpoll -m tcp -a $SLAVE_ADDR -r 100 -c 64 -t 4 -l $INTERVAL -o $TIMEOUT -p $PORT $IP
