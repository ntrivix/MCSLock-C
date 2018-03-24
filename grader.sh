#!/bin/bash


while true; do
	i="0"
	for i in `seq 1 70`;
	do
		TIME_NOW=$(date --u +%Y%m%d_%H%M%SZ)
		timeout 250s ./program > outputs/output${TIME_NOW}_${i}.txt 2>&1 &
	done
	sleep 200
done