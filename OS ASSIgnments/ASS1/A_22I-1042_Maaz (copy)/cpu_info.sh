#!/bin/bash

#cat /proc/cpuinfo
no_processor=$(grep -c "processor" /proc/cpuinfo)
total_cores=$(grep -c "core id" /proc/cpuinfo)
hyper_enable=$(grep -c "siblings" /proc/cpuinfo)
cores_with_hyper=$((no_processor / hyper_enable))
processor_model=$(grep "model name" /proc/cpuinfo | uniq | cut -d':' -f2 | sed 's/^[ \t]*//')

echo "Total Number of Processors: $no_processor" > CPU_info.txt
echo "Total Number of Physical Cores: $total_cores" >> CPU_info.txt
echo "Hyperthreading Enabled: $(($hyper_enable > 1))" >> CPU_info.txt
echo "Total Number of Cores with Hyperthreading: $cores_with_hyper" >> CPU_info.txt
echo "Processor Model: $processor_model" >> CPU_info.txt
