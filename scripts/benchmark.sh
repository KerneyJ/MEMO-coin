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
../bin/dsc wallet send multi 1 1 128000
date
