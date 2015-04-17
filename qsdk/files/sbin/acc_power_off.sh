#!/bin/bash

. /sbin/autelan_functions.sh
file_path=/root/onoff

get_onoff_log() {
	local ontime_file=/root/onoff/ap-on
	local ontime=$( cat ${ontime_file} |sed -n '$p' )       
	local offtime=$(get_time)

	write_ap_onoff_reason ${ontime} ${offtime} ${file_path}/ap-on-off-${offtime} ACC-OFF	
}

main() {
	touch /root/onoff/acc_off.txt
	get_onoff_log
}

main "$@"
