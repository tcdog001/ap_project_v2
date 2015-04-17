#!/bin/bash

. ${__ROOTFS__}/etc/jsock/msg/msg.in

#
# call by md
#
#$1:version
#$2:tftpname(include path)
#$3:after
#
main() {
	local version="$1"
	local tftpname="$2"
	local after="$3"

	jsock_md_send_check || {
		return ${e_bad_board}
	}

	if [ -z "${after}" ]; then
		after=40
	fi

	#
	# wait for ap startup
	#	todo: use keepalive trigger
	#
	sleep ${after}

	local body=$(printf ${jmsgbody_upgrade_request} ${version} ${tftpname})
	${__ROOTFS__}/etc/jsock/jmsg_asyn.sh "upgrade_request" "${body}"

	echo_logger "upgrade_request" "${body}"
}

main "$@"
