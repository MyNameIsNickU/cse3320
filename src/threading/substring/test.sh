#!/bin/bash
i=0
while [ $i -lt 20 ]
do
	./thread shakespeare.txt
        ./thread hamlet.txt
        ./thread small.txt
	((i=i+1))
done
