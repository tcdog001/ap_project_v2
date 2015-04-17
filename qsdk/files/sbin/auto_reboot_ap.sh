#!/bin/bash

. /sbin/autelan_functions.sh

off_delay() {
    ppp_json_string
    /bin/sleep 5
}

do_reboot() {
	# Record Timeout log
	local time=$(get_time)
	local log_path=/root/onoff
	local log_file=timeout-ap-on-off-
	local ontime=$(cat ${log_path}/ap-on |sed -n '$p')
	touch ${log_path}/acc_off.txt
	
	write_ap_onoff_reason "${ontime}" "${time}" "${log_path}/${log_file}${time}" "timeout"
	set_error_flag "${log_path}/${log_file}${time}" "Warn"
	# count 3g-flow
	off_delay
	sleep 5
	reboot
}

main() {
        # From power began to wait X hours
	local power_time=$1
	if [[ -z $1 ]];then
		power_time=43200
	fi
	sleep ${power_time}
	do_reboot
}

main "$@"
