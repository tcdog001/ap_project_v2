#!/bin/bash

. ${__ROOTFS__}/etc/jsock/jsock.in


usage() {
	echo "$0 usage:"
	echo "  $0 {name} {body}"
	echo "	  name: message name"
	echo "	  body: message body, must json format"
	echo
}

#
#$1:name
#$2:body...
#
main() {
	local argc=$#

	if ((argc<2)); then
		usage

		return ${e_inval}
	fi

	jmsg "${JSOCK_PEER}" "asyn" "$@"
}

main "$@"
