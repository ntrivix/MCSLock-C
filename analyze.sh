#!/bin/bash

invalid=0
all=0
notimeouts=0
segmentationfaults=0
deadlocks=0

for filename in outputs/*.txt; do
    res=$(< ${filename})
    #echo ${res}
    timeRegex="Work time is ([0-9\.]+).*"
    validMutesRegex="Mutex implementation is (not valid|valid)\."
    properTimeoutsRegex="There were ([0-9]*) proper \(timeout\) fails"
    locksAfterTimeoutRegex="There were ([0-9]*) locks after timeout"
    ttime=""
    tvalid="invalid"
    tproperTimeouts=""
    tlocksAfterTimeout=""
    if [[ $res =~ $timeRegex ]];
    then
        ttime="${BASH_REMATCH[1]}"
  	fi
  	if [[ $res =~ $validMutesRegex ]];
    then
        tvalid="${BASH_REMATCH[1]}"
  	fi
  	if [[ $res =~ $properTimeoutsRegex ]];
    then
        tproperTimeouts="${BASH_REMATCH[1]}"
  	fi
  	if [[ $res =~ $locksAfterTimeoutRegex ]];
    then
        tlocksAfterTimeout="${BASH_REMATCH[1]}"
  	fi
  	if [[ !(${tvalid} == "valid") ]];
  	then
        echo "${filename}"
  			echo "${tvalid}"
  			if [[ $res =~ "Segmentation fault" ]]; then
  				let "segmentationfaults++"
  				echo "SEG FAULT"
  			fi
        if [[ ${tvalid} == "invalid" ]]; then
          echo "NOT FINISHED!!!"
          let "deadlocks++"
        fi
  			echo "valid timeouts: ${tproperTimeouts}"
  			echo "locks after timeout: ${tlocksAfterTimeout}"
  			echo "---------------------------------------------------"
  			let "invalid++"
  			if [[ ${tproperTimeouts} == 0 ]];
  			then
  					let "notimeouts++"
  			fi
  	fi
  	let "all++"
done
echo "total: ${all}"
echo "invalid: ${invalid}"
echo "no timeouts: ${notimeouts}"
echo "deadlocks: ${deadlocks}"