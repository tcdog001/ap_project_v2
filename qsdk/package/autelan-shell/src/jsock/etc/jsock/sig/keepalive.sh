#!/bin/bash

. ${__ROOTFS__}/etc/jsock/sig/sig.in

send_register_info=0
interval=10
peer_dead_threshold=3
open_alarm=0

main() {
	# check board type
	local board_type=$(get_board_type)

	if [ ! -f ${file_keepalive_state} ];then
		echo 0 > ${file_keepalive_state}
	fi

	while true
	do
		#
		# check peer is alive or not
		#
		local kpalive_state="`cat ${file_keepalive_state}`"
		if [ -z "${kpalive_state}" ];then
				kpalive_state=0
		fi
		#echo "kpalive_state:${kpalive_state}"
		if [ ${open_alarm} -eq 0 ];then
			if [ ${kpalive_state} -gt ${peer_dead_threshold} ];then
				jsig_logger "not recv peer msg for a long time, peer dead, open alarm led"
				#
				# open alarm led
				#
				if [ "${board_type}" == "ap" ];then
						echo 1 > /sys/class/leds/db120\:green\:alarm/brightness
				fi
				open_alarm=1
			fi
		else
			if [ ${kpalive_state} -le ${peer_dead_threshold} ];then
				jsig_logger "recv peer msg after peer dead, close alarm led"
				#
				# close alarm led
				#
				if [ "${board_type}" == "ap" ];then
						echo 0 > /sys/class/leds/db120\:green\:alarm/brightness
				fi
				open_alarm=0
			fi
		fi
		kpalive_state=$((kpalive_state+1))
		echo ${kpalive_state} > ${file_keepalive_state}

		if [ "${board_type}" == "ap" ];then
			#
			# if first keepalive on route, also send register info
			#
			#if ((send_register_info<10)); then
				${__ROOTFS__}/etc/jsock/msg/getsysinfo.sh
			#	((send_register_info++))
			#fi

			${__ROOTFS__}/etc/jsock/jsig.sh asyn keepalive
		fi

		#
		# read and add seq
		#
		local seq=$(readnumberfile_andadd ${file_keepalive_seq} 1)

		sleep ${interval}
	done
}

main "$@"
