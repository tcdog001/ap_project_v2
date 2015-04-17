#!/bin/bash

#
# autelan scripts log path
#
#DEBUG_LOG_LOCAL=/tmp/debug.log
DEBUG_LOG_LOCAL=/dev/null

#
# default custom parameters
#
readonly DEF_SSID1=9797168.com
readonly DEF_SSID0=config.${DEF_SSID1}
readonly DEF_LMS=atbus.autelan.com
readonly DEF_PORTAL=9797168.com
readonly OEM_MD_FLAG=/tmp/.oem_md.json

#
# ppp log path
#
readonly PPPPATH=/root/ppp
readonly DOWNFLOWPATH=${PPPPATH}/down_all
readonly UPFLOWPATH=${PPPPATH}/up_all
readonly PPPTMPPATH=/tmp/.ppp
readonly DOWNFLOWTMP=${PPPTMPPATH}/down_now
readonly UPFLOWTMP=${PPPTMPPATH}/up_now
readonly DMESGPATH=/root/log/init
readonly SYSLOGDPATH=/root/log/ulog
readonly LASTSYNTIME=/root/log/ulog/timesyn
readonly SYNTIME=/tmp/timesyn
readonly ICCID_PATH=/root/ppp/iccid

readonly INTERFACE_3G_FIREWALL_FLAG=/tmp/.evdo
delete_3g_firewall_flag() {
	rm ${INTERFACE_3G_FIREWALL_FLAG}
}
create_3g_firewall_flag() {
	touch ${INTERFACE_3G_FIREWALL_FLAG}
}

ppp_json_string() {
	local time=$(get_time)
	local DOWNFLOW=$(cat ${DOWNFLOWPATH} 2>> ${DEBUG_LOG_LOCAL})
	local UPFLOW=$(cat ${UPFLOWPATH} 2>> ${DEBUG_LOG_LOCAL})
	local EDTIME=$(cat ${PPPTMPPATH}/endtime 2>> ${DEBUG_LOG_LOCAL})
	local STTIME=$(cat ${PPPTMPPATH}/starttime_all 2>> ${DEBUG_LOG_LOCAL})
	local JSstring={\"device\":\"cdma\",\"starttime\":\"${STTIME}\",\"endtime\":\"${EDTIME}\",\"up\":\"${UPFLOW}\",\"down\":\"${DOWNFLOW}\"}

	echo ${JSstring} >> ${PPPPATH}/3g-flow-${time}.log
	rm -f ${UPFLOWPATH} ${DOWNFLOWPATH}
}

get_time() {
	local Time=$(date -I "+%F-%H:%M:%S")
	#Time=$(/bin/date -Iseconds | /bin/sed 's/:/-/g;s/T/-/g;s/+0800//g')
	echo ${Time}
}

#
# $1: file, content time like 2014-12-10 15:37:37
# output time like 2014-12-10-15:37:37
#
modify_time() {
	local file="$1"
	sed -i 's/ /-/g' ${file}
}

get_syntime() {
	local file=${SYNTIME}
	local time=""

	if [ -f ${file} ]; then
		modify_time ${file}
		cat ${file}
	else
		time=$(get_time)
		echo ${time}
	fi
}

get_last_syntime() {
	local file=${LASTSYNTIME}
	local time=""

	if [ -f ${file} ]; then
		modify_time ${file}
		cat ${file}
	else
		time=$(get_time)
		echo ${time}
	fi
}

save_init_log() {
	local time=$(get_syntime)
	local path=${DMESGPATH}

	# save init log
	dmesg > ${path}/init-${time}
}

save_last_syslog() {
	local time=$(get_last_syntime)
	local file=${SYSLOGDPATH}/messages
	
	# save last syslogd message
	[[ -f ${file} ]] && mv ${file} ${SYSLOGDPATH}/ulog-${time}
}

restart_prepare() {
	local time=$(get_time)
	local cmd="/etc/jsock/jmsg.sh asyn acc_off {\"date\":\"${time}\"}"
	local ontime_file=/root/onoff/ap-on

	acc_power_off.sh &
	upload_vcc.sh &
	sleep 5

	echo "$0: ${cmd}"
	${cmd} &

	ppp_json_string

	mv /tmp/log/messages ${SYSLOGDPATH}/
	mv ${SYNTIME} ${SYSLOGDPATH}/
	sleep 5
}

#
# $1: R_path
# $2: M_tmppath
# $3: M_path
# $4: M_ip
# $5: file
# $6: filerename
#
tftp_RtoM() {
	local R_path="$1"
	local M_tmppath="$2"
	local M_path="$3"
	local M_ip="$4"
	local file="$5"
	local filerename="$6"

	tftp -p -l ${R_path}/${file} -r ${M_tmppath}/${filerename} ${M_ip} 2>> ${DEBUG_LOG_LOCAL}
	if [[ $? = 0 ]];then
		echo "$0: ${file} TFTP OK" 2>> ${DEBUG_LOG_LOCAL}
		/etc/jsock/jcmd.sh syn mv ${M_tmppath}/${filerename} ${M_path}/ &
		rm -rf ${R_path}/${file}
		return 0
	else
		echo "$0: ${file} TFTP NOK" 2>> ${DEBUG_LOG_LOCAL}
		return 1
	fi
}

get_3g_model() {
	local model=$(cat /tmp/3g_model 2> /dev/null)
	echo ${model}
}

get_sim_iccid() {
	local model=$(get_3g_model)
	local sim_iccid=""

	case ${model} in
		"C5300V")
			sim_iccid=$(at_ctrl at+iccid | awk '/SCID:/{print $2}' 2> /dev/null)			
			;;
		"DM111")
			sim_iccid=$(at_ctrl at+iccid | awk '/ICCID:/{print $2}' 2> /dev/null)
			;;
		"SIM6320C")
			sim_iccid=$(at_ctrl at+ciccid | awk '/ICCID:/{print $2}' 2> /dev/null)
			;;
		"MC271X")
			sim_iccid=$(at_ctrl at+zgeticcid | awk '/ZGETICCID/{print $2}' 2> /dev/null)
			;;
		*)
			logger -t $0 "Model=${model} Not Support" 
			;;
	esac
	echo ${sim_iccid}
}
get_iccid() {
	cat ${ICCID_PATH}
}
get_hdrcsq() {
	local model=$(get_3g_model)
	local hdrcsq=""

	case ${model} in
		"SIM6320C")
			hdrcsq=$(at_ctrl at^hdrcsq |awk -F ': ' '/hdrcsq/{print $2}' 2>/dev/null)
			;;
		*)
			hdrcsq=$(at_ctrl at^hdrcsq |awk -F ':' '/HDRCSQ/{print $2}' 2>/dev/null)
			;;
	esac
	echo ${hdrcsq}
}
get_meid_of3g() {
	local model=$(get_3g_model)
	local meid=""
	
	case ${model} in
		"SIM6320C")
			meid=$(at_ctrl ati | awk -F ': 0x' '/MEID/{print $2}' 2>/dev/null)
			;;
		*)
			meid=$(at_ctrl at^meid | awk -F '0x' '/0x/{print $2}'| sed -n '1p' 2>/dev/null)
			;;
	esac
	echo ${meid}
}
report_meid_of3g() {
	local meid_path=/root/ppp/meid
	local meid=$(get_meid_of3g)
	
	if [[ ${meid} ]]; then
		echo ${meid} > ${meid_path}
		logger -t $0 "get meid=${meid}"
	else
		meid=$(cat ${meid_path} 2> /dev/null)
		logger -t $0 "fake meid=${meid}"
	fi

	[[ -z ${meid} ]] && meid="INVALID DATA"
	echo ${meid}
}
report_sim_iccid() {
	local iccid_success=/root/ppp/iccid_success
	local iccid_succ_num=""
	local iccid_fail=/root/ppp/iccid_fail
	local iccid_fail_num=""
	local sim_iccid=$(get_sim_iccid)
	
	if [[ ${sim_iccid} ]]; then
		echo ${sim_iccid} > ${ICCID_PATH}
		iccid_succ_num=$(cat ${iccid_success} 2> /dev/null)
		((iccid_succ_num++))
		echo ${iccid_succ_num} > ${iccid_success}
		logger -t $0 "get iccid=${sim_iccid}"
	else
		sim_iccid=$(cat ${ICCID_PATH} 2> /dev/null)
		iccid_fail_num=$(cat ${iccid_fail} 2> /dev/null)
		((iccid_fail_num++))
		echo ${iccid_fail_num} > ${iccid_fail}
		logger -t $0 "fake iccid=${sim_iccid}"
	fi

	[[ -z ${sim_iccid} ]] && sim_iccid="INVALID DATA"
	echo ${sim_iccid}
}
get_company_of3g() {
	local model=$(get_3g_model)
	local company_of3g=""

	case ${model} in
		"DM111")
			company_of3g="Wistron NeWeb Corp."
			;;
		"SIM6320C")
			company_of3g="SIMCOM INCORPORATED"
			;;
		"MC271X")
			company_of3g="ZTEMT INCORPORATED"
			;;
		*)
			#company_of3g=$(at_ctrl AT+CGMI 2> /dev/null)
			logger -t $0 "Model=${model} Not Support" 
			;;
	esac
	echo ${company_of3g}
}
get_3g_net() {
	local sim_iccid=$(report_sim_iccid)
	local code=$(echo ${sim_iccid} | awk -F '' '{print $5$6}')
	local net=""
	
	[[ "${code}" = "01" ]] && net=WCDMA
	[[ "${code}" = "03" ]] && net=CDMA2000
	[[ "${code}" = "00" ]] && net=GSM/TD-SCDMA
	[[ "${code}" = "02" ]] && net=GSM/TD-SCDMA
	[[ "${code}" = "07" ]] && net=TD-SCDMA
	echo ${net}
}
#
# $1: option
#
get_option_value() {
	local option=$1
	local result=$(uci get ${option} 2> /dev/null)
	
	echo ${result}
}
#
# $1: option
# $2-$#: value to be checked
#
check_option_value() {
	local option=$1; shift 1
	local value="$@"
	local value_get=$(get_option_value ${option})
	
	[[ -z ${option} || -z ${value} || -z ${value_get} ]] && return 0
	if [[ ${value} == ${value_get} ]]; then
		return 0
	else
		return 1
	fi
}
#
# $1: option
# $2-$#: value to be set
#
set_option_value() {
	local option=$1; shift 1
	local value="$@"
	local check=0
	local cmd="uci set ${option}=\"${value}\""
	
	[[ -z ${option} || -z ${value} ]] && return 0
	check_option_value ${option} ${value}; check=$?
	if [[ ${check} -eq 1 ]]; then
		logger -t $0 ${cmd}
		eval ${cmd}
		return 1
	else
		return 0
	fi
}
#
# $1: [<config>]
#
commit_option_value() {
	local config=$1
	local cmd="uci commit ${config}"
	
	logger -t $0 ${cmd}
	${cmd}
}
#
# $1: log tag name
# $2: log content
#
exit_log() {
	local script=$1; shift 1
	local string="$@"
	
	logger -t ${script} ${string}
	exit
}
#
# $1: key
# $2: value; shift 2
# $@: json string
#
add_json_string() {
	local key="$1"
	local value="$2"; shift 2
	local str_old="$@"
	local str_new="\"${key}\":\"${value}\""
	local str_entirety
	
	if [[ ${str_old} ]]; then
		str_entirety="${str_old},${str_new}"
	else
		str_entirety=${str_new}
	fi                                      

	echo ${str_entirety}
}
#
# $1: ontime
# $2: offtime
# $3: filepath
# $@: reason
#
write_ap_onoff_reason() {
	local ontime="$1"
	local offtime="$2"
	local filepath="$3"; shift 3
	local reason="$@"

	printf '{"ontime":"%s","offtime":"%s","offreason":"%s"}\n' \
		"${ontime}"  \
		"${offtime}" \
		"${reason}" > ${filepath}
}
#
# $@: error_level
#
set_error_flag() {
	local file_name=$1; shift 1
    local error_level="$@"
	local error_time=$(cat ${file_name} |jq -j '.offtime |strings')
	local log_substance=$(cat ${file_name} |sed 's/{//g;s/}//g')          
	local ap_mac=$(showsysinfo |awk '/MAC/{print $1}' |sed 's/MAC://g')
	local error_reason=$(cat ${file_name} |jq -j '.offreason |strings')
	local file_error=/root/onoff/ap_${error_reason}_${error_time}

	printf '{\"ap_mac\":\"%s\",%s,\"log_level\":\"%s\"}\n'	\
		"${ap_mac}"			\
		"${log_substance}" 	\
		"${error_level}" > ${file_error}
}

