#!/bin/bash
declare -i i=1
until ((30<i))
    do 
    let i++
    sleep 1
    echo "1"
done
exit 0