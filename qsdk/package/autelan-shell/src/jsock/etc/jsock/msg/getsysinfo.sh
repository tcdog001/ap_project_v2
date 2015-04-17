#!/bin/bash
	
. ${__ROOTFS__}/etc/jsock/msg/msg.in

main() {
	jsock_ap_send_check || {
		return ${e_bad_board}
	}

	local json="$(${__ROOTFS__}/usr/sbin/getapinfo.sh)"
	${__ROOTFS__}/etc/jsock/jmsg.sh asyn getsysinfo "${json}"
	jmsg_logger "ap send msg json:${json}"
}

main "$@"
