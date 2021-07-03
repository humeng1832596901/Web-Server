#!/bin/bash

for i in $(ls -F |grep "/$")
do
    cd $i
    sh create.sh
    cd ..
done

