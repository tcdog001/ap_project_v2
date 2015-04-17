#!/bin/bash

. /sbin/autelan_functions.sh
file_path=/root/onoff

get_onoff_log() {
	local file_on=ap-on
	local file_off=ap-off
	local ontime=$( cat ${file_path}/${file_on} |sed -n '$p' )
	local offtime=$(cat ${file_path}/${file_off})
	local line=$( grep -n "" ${file_path}/${file_on} |wc -l )
	local del_line=$(awk 'BEGIN{printf("%d",'$line'-'2')}')
	
	if [[ ${line} -gt 2 ]];then                                     
		sed -e "1,${del_line}"d ${file_path}/${file_on} -i 2>/dev/null
	fi

	if [[ -e ${file_path}/acc_off.txt ]];then                              
		rm -rf ${file_path}/acc_off.txt;
		return 0;                             
	else 
		write_ap_onoff_reason ${ontime} ${offtime} ${file_path}/ap-on-off-${offtime} DROP-OFF	
		set_error_flag	"${file_path}/ap-on-off-${offtime}" "Warn"
	fi
}

main() {
#	sleep 5
	get_onoff_log
}

main "$@"
