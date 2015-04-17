#!/bin/bash       

. /sbin/autelan_functions.sh
offtime_file=/root/onoff/ap-off
ontime_file=/root/onoff/ap-on

get_ontime() {
	local time=""

	sleep 40
	time=$(get_time)
	echo ${time} >>${ontime_file} 2>/dev/null
	
	sleep 5
	time=$(get_time)
	echo ${time} >${offtime_file} 2>/dev/null
}

get_offtime() { 
	local time=""
	while :
	do
		time=$(get_time)
		echo ${time} >${offtime_file} 2>/dev/null
		sleep 60
	done
}

main() {
	get_ontime
	get_offtime
}

main "$@"

