#!/bin/bash

num_client=20

#loop

for((i=1; i<=$num_client; i++))
do
    ./client &

    echo "coneected to server $i"
    

    sleep 1

 done


 wait


 echo "all client have finished."