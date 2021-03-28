#!/bin/bash
i=0
while [ $i -lt 20 ]
do
	./queue message.txt
	((i=i+1))
done
