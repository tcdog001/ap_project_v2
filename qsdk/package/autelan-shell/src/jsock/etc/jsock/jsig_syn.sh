#!/bin/bash

. ${__ROOTFS__}/etc/jsock/jsock.in


usage() {
	echo "$0 usage:"
	echo "  $0 {name}"
	echo "	  name: signal name"
	echo
}

#
#$1:name
#
main() {
	local argc=$#

	if ((1!=argc)); then
		usage

		return ${e_inval}
	fi

	jsig "${JSOCK_PEER}" "syn" "$@"
}

main "$@"
