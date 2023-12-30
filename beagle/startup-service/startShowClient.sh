#!/bin/sh
SLEEPSEC=2
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
		
        /usr/bin/amixer set Speaker 86%

        echo "start" | tee /sys/class/remoteproc/remoteproc1/state
        echo "start" | tee /sys/class/remoteproc/remoteproc2/state

		echo "Starting the client" | tee -a /root/mystartupmessage
		/root/startup/ShowClient /media/client.cfg
		exit 0
	fi
else
	echo "PRU was never ready, loopcount was: $loopcount" | tee -a /root/mystartupmessage
	exit 1
fi



