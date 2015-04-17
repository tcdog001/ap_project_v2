#!/bin/bash

. ${__ROOTFS__}/etc/jsock/msg/msg.in

#
# call by ap
#
#$1:version
#$2:reboot_after
#[$3:mode active/passive]
#
main() {
	local version="$1"
	local after="$2"
	local mode="$3"

	if [ -z "${mode}" ]; then
		mode="${upgrade_response_mode_deft}"
	fi

	jsock_ap_send_check || {
		return ${e_bad_board}
	}

	echo_logger "upgrade_response" \
		"ap is upgrading by mode:${mode}, md should reboot after ${after}"

	local body=$(printf ${jmsgbody_upgrade_response} ${version} ${after} ${mode})
	${__ROOTFS__}/etc/jsock/jmsg_asyn.sh "upgrade_response" "${body}"
}

main "$@"
