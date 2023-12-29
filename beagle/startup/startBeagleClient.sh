#!/bin/sh
SLEEPSEC=8
MAXLOOP=35
##################################################
loopcount=0
PRUREADY=0

#sleep 60

while [ $loopcount  -lt $MAXLOOP ]
do


#### Waiting on pru

if [ -e "/sys/class/remoteproc/remoteproc2/firmware" ]
then
		echo "Pru ready... $loopcount" | tee -a /root/mystartupmessage
		sleep 1 
		PRUREADY=1;
		break
else
		sleep $SLEEPSEC
		echo "Waiting for pru... $loopcount" | tee -a /root/mystartupmessage
		loopcount=`expr $loopcount + 1`

fi

done

if [ $PRUREADY  -eq 1 ]
then
	loopcount=0

	STARTLIGHTS=0

	while [ $loopcount  -lt $MAXLOOP ]
	do
		mount /dev/mmcblk0p1 /media
		mountpoint -q /media
		temp=$?
		if [ $temp -eq  0 ]
		then
			STARTLIGHTS=1
		
			/usr/bin/amixer set Speaker 86%

			echo "stop" | tee /sys/class/remoteproc/remoteproc1/state
			echo "stop" | tee /sys/class/remoteproc/remoteproc2/state

			echo "DIYBLIGHTS-fw" | tee /sys/class/remoteproc/remoteproc1/firmware
			echo "DIYBLIGHTS-fw" | tee /sys/class/remoteproc/remoteproc2/firmware

			echo "start" | tee /sys/class/remoteproc/remoteproc1/state
			echo "start" | tee /sys/class/remoteproc/remoteproc2/state

			break
		else
			sleep $SLEEPSEC
			echo "Waiting for '/media' to mount... $loopcount" | tee -a /root/mystartupmessage
			loopcount=`expr $loopcount + 1`
		fi
	done
	if [ $STARTLIGHTS  -eq 1 ]
	then
		echo "Starting the client" | tee -a /root/mystartupmessage
		#/root/startup/ShowClient /media/Config.cfg
		exit 0
	else 
		echo "'/media' never was mounted, loopcount was: $loopcount" | tee -a /root/mystartupmessage
		exit 1
	fi
	exit 0
else 
	echo "PRU was never ready, loopcount was: $loopcount" | tee -a /root/mystartupmessage
	exit 1
fi



