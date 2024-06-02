#!/bin/bash

read -p "Enter the number of files you want to create: " n

mkdir -p TestData

for ((i=1; i<=n; i++));
do
   echo "Temp data. This is assignment 01. Subject OS. Entry number $i" > TestData/f$i
done

