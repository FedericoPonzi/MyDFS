#!/bin/sh


x=1
while [ $x -le 1000 ]
do
  echo "Ciaoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo" >> /tmp/test.txt
  x=$(( $x + 1 ))
done
