#!/bin/sh

generale="/home/federicoponzi/Progetto"

$generale/Server/Server &

echo "OPE file 2" > netcat localhost 9000
