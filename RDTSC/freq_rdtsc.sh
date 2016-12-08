#!/bin/bash

#
# NOTE that my platform is Intel Xeon CPU E5-2680 v3.
# Its frequency ranges from 1.20GHz to 2.50GHz (without
# enabling Turboboost.
#

sudo cpufreq-set -c 0 -f 1.20GHz
sleep 10
echo "****** Low frequency ******"
cnt=0
while [ $cnt -lt 10 ]
do
    ./rdtsc
    let cnt=$cnt+1
done

sudo cpufreq-set -c 0 -f 2.50GHz
sleep 10
echo "****** High frequency ******"
cnt=0
while [ $cnt -lt 10 ]
do
    ./rdtsc
    let cnt=$cnt+1
done
