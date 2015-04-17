#!/bin/bash

. ${__ROOTFS__}/etc/jsock/jsock.in


usage() {
	echo "$0 usage:"
	echo "  $0 {mode} {name}"
	echo "	  mode: asyn/ack/syn"
	echo "	  name: signal name"
	echo
}

#
#$1:mode
#$2:name
#
main() {
	local argc=$#

	if ((2!=argc)); then
		usage

		return ${e_inval}
	fi

	jsig "${JSOCK_PEER}" "$@"
}

main "$@"
