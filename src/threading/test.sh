#!/bin/bash
i=0
while [ $i -lt 20 ]
do
	./thread hamlet.txt
	((i=i+1))
done
