#!/bin/sh


# start the prus
echo "start" | tee /sys/class/remoteproc/remoteproc1/state
echo "start" | tee /sys/class/remoteproc/remoteproc2/state

# configure the leds
echo "none"  | tee  /sys/class/leds/beaglebone:green:usr0/trigger
echo "none"  | tee  /sys/class/leds/beaglebone:green:usr1/trigger
echo "none"  | tee  /sys/class/leds/beaglebone:green:usr2/trigger
echo "none"  | tee  /sys/class/leds/beaglebone:green:usr3/trigger

echo "0"     | tee  /sys/class/leds/beaglebone:green:usr0/brightness
echo "0"     | tee  /sys/class/leds/beaglebone:green:usr1/brightness
echo "0"     | tee  /sys/class/leds/beaglebone:green:usr2/brightness
echo "0"     | tee  /sys/class/leds/beaglebone:green:usr3/brightness
