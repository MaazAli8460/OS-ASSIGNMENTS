#!/bin/bash

if [ $# -ne 1 ];
then 
    echo "Error: No Args given. Try again"
    exit 1
fi

mkdir -p myTrash

cp "$1" myTrash/

rm "$1"
