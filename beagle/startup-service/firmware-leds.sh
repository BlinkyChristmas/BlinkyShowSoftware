#!/bin/sh
SLEEPSEC=2
MAXLOOP=35
##################################################
loopcount=0
PRUREADY=0

# Waiting on pru (it seems to always take 30 seconds)
while [ $loopcount  -lt $MAXLOOP ]
do
if [ -e "/sys/class/remoteproc/remoteproc2/firmware" ]
then
                echo "Pru ready... $loopcount" | tee -a /root/mystartupmessage
                PRUREADY=1;
                break
else
                sleep $SLEEPSEC
                echo "Waiting for pru... $loopcount" | tee -a /root/mystartupmessage
                loopcount=`expr $loopcount + 1`

fi
done

if [ $PRUREADY -eq 1 ]
then
        echo "Starting the pru" | tee -a /root/mystartupmessage
# start the prus
        echo "start" | tee /sys/class/remoteproc/remoteproc1/state
        echo "start" | tee /sys/class/remoteproc/remoteproc2/state

# configure the leds
        echo "none"  | tee  /sys/class/leds/beaglebone\:green\:usr0/trigger
        echo "none"  | tee  /sys/class/leds/beaglebone\:green\:usr1/trigger
        echo "none"  | tee  /sys/class/leds/beaglebone\:green\:usr2/trigger
        echo "none"  | tee  /sys/class/leds/beaglebone\:green\:usr3/trigger

        echo "0"     | tee  /sys/class/leds/beaglebone\:green\:usr0/brightness
        echo "0"     | tee  /sys/class/leds/beaglebone\:green\:usr1/brightness
        echo "0"     | tee  /sys/class/leds/beaglebone\:green\:usr2/brightness
        echo "0"     | tee  /sys/class/leds/beaglebone\:green\:usr3/brightness
fi
