#!/bin/bash
#author: Nina Zukowska
#id: 148278
##the killer function is accually just here as a backup
##the time the program runs is specified by the 'running' variable - i did't want to cause any problems and like this its also more conveniant in my opinion

killer() {
    
    sleep 20
    echo "STARTING MASS MURDER"
    killall producer
    killall consumer
    echo "MASS MURDER COMMITED"
}

#
WAITING=100
N=10
PRODUCERS=5
CONSUMERS=$((N-PRODUCERS))
RUNNING=10

echo "The minimum sleepeing time of a process is $WAITING"
echo "The total number of consumers is $CONSUMERS, while the total number of producers is $PRODUCERS"
cc -Wall -o producer producer.c -lrt -lpthread
cc -Wall -o consumer consumer.c -lrt -lpthread

for(( i=1; i <= $PRODUCERS; ++i ))
do 
	./producer $N $WAITING $RUNNING $CONSUMERS &
done

for(( i=1; i <= $CONSUMERS; ++i ))
do
	./consumer $N $WAITING $RUNNING $PRODUCERS &
done
wait 






