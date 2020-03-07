#!/bin/bash

PORT=$1
SERVERNAME=$2
NUM_CLIENTS=$3

for i in $NUM_CLIENTS
do
    ./client $2 $1 &
done
