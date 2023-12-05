#!/bin/bash

../bin/dsc blockchain &
../bin/dsc pool > /dev/null &

for i in {1..12}
do
    ../bin/dsc validator > /dev/null &
done

../bin/dsc metronome > /dev/null &

sleep 5

printf "starting benchmark\n"
date
for i in {1..128000}
do
    ../bin/dsc wallet send 1 1
#    if [ $(($i % 1000)) -eq 0 ]; then
#        printf "submitted transaction %d\n" $i
#    fi
done
date
