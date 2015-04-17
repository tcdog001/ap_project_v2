#!/bin/bash

. /sbin/autelan_functions.sh

get_cpu_Frequency() {
	cpuFrequency=$(cat /proc/cpuinfo |awk -F ':' '/BogoMIPS/{print $2}')
	if [[ -z "${cpuFrequency}" ]];then
		cpuFrequency="278.93"
	fi
	echo "${cpuFrequency}"
}

get_cpu_use() {
	cpuUse=$(top -n 1 |awk '{print $7}' |sed '1,4d' |awk '{sum += $1};END {print sum}')
	if [[ -z "${cpuUse}" ]];then
		cpuUse="NULL"
	fi
	echo "${cpuUse}%"
}

get_memory_size() {
	memorySize=$(free |awk -F ' ' '/Mem:/{print $2}')
	if [[ -z "${memorySize}" ]];then
		memorySize="126467"
	fi
	echo "${memorySize}"
}

get_memory_use() {
	memoryUse=$(free |awk -F ' ' '/Mem:/{print $3}')
	memoryUsage1=$(awk 'BEGIN{printf "%.2f\n",'$memoryUse'/'$memorySize'*100}')
	memoryUsage="$(echo ${memoryUsage1}%)"
	if [[ -z "${memoryUsage}" ]];then
		memoryUsage="0%"
	fi
	echo "${memoryUsage}"
}

get_board_temperature() {
	boardTemperature=``
	if [[ -z "$boardTemperature" ]];then
		boardTemperature="0"
	fi
	echo "$boardTemperature"
}

get_networkWIFI0_state() {
	networkstation1=$(iw dev wlan0-1 station dump |grep -r "Station")
	network1=$(iw dev wlan0-1 station dump |grep -r "inactive")
	networkstation2=$(echo $network1 |awk '{gsub(/ /,"")}1')
	networkcardLow="${networkstation1}${networkstation2}"
	if [[ -z "${networkcardLow}" ]];then
		networkcardLow="WIFI0 NO STA"
	fi
	echo "${networkcardLow}"
}

get_APLow_info() {
	APLow=$(iw dev |grep -i 'channel' |sed -n '$p')
	APLowinfo=$(echo $APLow |awk '{gsub (/ /,"")}1')
	if [[ -z "${APLowinfo}" ]];then
			APLowinfo="WIFI0 DOWN"
	fi
	echo "${APLowinfo}"
}

get_APLow_STAnum() {
	APLowSTAnum=$(iw dev wlan0-1 station dump |grep -i 'station' |wc -l)
	if [[ -z "${APLowSTAnum}" ]];then
		APLowSTAnum="WIFI0 NO STA"
	fi
	echo "${APLowSTAnum}"
}

get_networkWIFI1_state() {                                        
        networkstation1=$(iw dev wlan1 station dump |grep -r "Station")
        network2=$(iw dev wlan1 station dump |grep -r "inactive")
        networkstation2=$(echo $network2 |awk '{gsub(/ /,"")}1')
        networkcardHigh="${networkstation1}${networkstation2}"
        if [[ -z "${networkcardHigh}" ]];then
                networkcardHigh="WIFI1 NO STA"                                            
        fi                                                                                
        echo "${networkcardHigh}"
}

get_APHigh_info() {
	APHigh=$(iw dev |grep -i 'channel' |sed -n '1p')
	APHighinfo=$(echo $APHigh |awk '{gsub (/ /,"")}1')
	if [[ -z "${APHighinfo}" ]];then
			APHighinfo="WIFI1 DOWN"
	fi
	echo "${APHighinfo}"
}

get_APHigh_STAnum() {
	APHighSTAnum=$(iw dev wlan1 station dump |grep -i 'station' |wc -l)
	if [[ -z "${APHighSTAnum}" ]];then
		APHighSTAnum="WIFI1 NO STA"
	fi
	echo "${APHighSTAnum}"
}

get_GPS_Satellite() {
	GPSSatellite=$(cat /tmp/gps_info |grep -i "GPGGA" |awk -F ',' '{print $8}' |sed -n '$p')
	if [[ -z "${GPSSatellite}" ]];then
		GPSSatellite="00"
	fi
	echo "${GPSSatellite}"
}

get_GPS_Location() {
	GPSLocation=$(cat /tmp/gps_info |grep -i "GPGGA" |awk -F ',' '{print $4$3$6$5}' |sed -n '$p')
	if [[ -z "${GPSLocation}" ]];then
		GPSLocation="GPS Location FAIL"                                                    
	fi                                     
	echo "${GPSLocation}"
}

get_GPS_Height() {
	GPSHeight=$(cat /tmp/gps_info |grep -i "GPGGA" |awk -F ',' '{print $10}' |sed -n '$p')
	if [[ -z "${GPSHeight}" ]];then
		GPSHeight="GPS Height FAIL"
	fi
	echo "${GPSHeight}"
}

get_GPS_Velocity() {
	GPSVelocity=$(cat /tmp/gps_info |grep -i "GPRMC" |awk -F ',' '{print $8}' |sed -n '$p')
	if [[ -z "${GPSVelocity}" ]];then
		GPSVelocity="GPS Velocity FAIL"
	fi
	echo "${GPSVelocity}"
}

get_GPS_Time() {
#	GPSTime=`cat /tmp/gps_info |grep -i "GPGGA" |awk -F ',' '{print $2}' |sed -n '$p'`
	local gps_file=/tmp/gps_info

	gps_year=$(cat /tmp/gps_info |awk -F ',' '/GPRMC/{print $10}' |sed -n '$p')
	gps_Y=$(echo ${gps_year} |awk -F '' '{print $5$6}')
	gps_M=$(echo ${gps_year} |awk -F '' '{print $3$4}')
	gps_D=$(echo ${gps_year} |awk -F '' '{print $1$2}')

	gps_strings=$( cat ${gps_file} |awk -F ',' '/GPGGA/{print $2}' |sed -n '$p' |awk -F '.' '{print $1}' )
	gps_h=$(echo ${gps_strings} |awk -F '' '{print $1$2}')
	gps_m=$(echo ${gps_strings} |awk -F '' '{print $3$4}')
	gps_s=$(echo ${gps_strings} |awk -F '' '{print $5$6}')
	CN_h=$(awk 'BEGIN{printf("%d",'${gps_h}'+'8')}')
	
	GPSTime=$(echo 20${gps_Y}-${gps_M}-${gps_D}-${CN_h}:${gps_m}:${gps_s})

	if [[ -z "${gps_h}" ]];then
		GPSTime="GPS Time FAIL"
	fi
	echo "${GPSTime}"
}

get_3G_Connection() {
	Connection3G=$(ifconfig 3g-evdo |grep addr 2>/dev/null)
	if [[ -z "${Connection3G}" ]];then
		Connection3G="NO"
	else
		Connection3G="YES"
	fi	
	echo "${Connection3G}"
}

get_3G_Signal() {
	Signal3G=$(get_hdrcsq)
	if [[ -z "${Signal3G}" ]];then
		Signal3G="0"
	fi
	echo "${Signal3G}"
}

get_APinfo_News() {                         
	local cpuFrequency=$(get_cpu_Frequency)
	local cpuUse=$(get_cpu_use)
	local memorySize=$(get_memory_size)
	local memoryUse=$(get_memory_use)
	local boardTemperature=$(get_board_temperature)
	local networkWIFI0=$(get_networkWIFI0_state)
	local networkWIFI1=$(get_networkWIFI1_state)
	local APLowinfo=$(get_APLow_info)
	local APLowSTAnum=$(get_APLow_STAnum)
	local APHighinfo=$(get_APHigh_info)
	local APHighSTAnum=$(get_APHigh_STAnum)
	local GPSSatellite=$(get_GPS_Satellite)
	local GPSLocation=$(get_GPS_Location)
	local GPSHeight=$(get_GPS_Height)
	local GPSVelocity=$(get_GPS_Velocity)
	local GPSTime=$(get_GPS_Time)
	local Connection3G=$(get_3G_Connection)
	local Signal3G=$(get_3G_Signal)
 
	printf '{"APcpuFrequency":"%s","APcpuUse":"%s","APmemorySize":"%s","APmemoryUse":"%s","APboardTemperature":"%s","networkWIFI0":"%s","APWIFI0info":"%s","APWIFI0STAnum":"%s","networkWIFI1":"%s","APWIFI1info":"%s","APWIFI1STAnum":"%s","GPSSatellite":"%s","GPSLocation":"%s","GPSHeight":"%s","GPSVelocity":"%s","GPSTime":"%s","Connection3G":"%s","Signal3G":"%s"}\n'  \
		"${cpuFrequency}"	\
		"$cpuUse"		\
		"$memorySize"		\
		"$memoryUse"		\
		"$boardTemperature"	\
		"$networkWIFI0"		\
		"$APLowinfo"		\
		"$APLowSTAnum"		\
		"$networkWIFI1"         \
		"$APHighinfo"		\
		"$APHighSTAnum"		\
		"$GPSSatellite"		\
		"$GPSLocation"		\
		"$GPSHeight"		\
		"$GPSVelocity"		\
		"$GPSTime"		\
		"$Connection3G"		\
		"$Signal3G"
}

main() {
	get_APinfo_News >/tmp/getsysinfo_AP.json
}

main "$@"
