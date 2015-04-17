#!/bin/bash

. ${__ROOTFS__}/etc/jsock/jsock.in


usage() {
	echo "$0 usage:"
	echo "  $0 {mode} {name} {body}"
	echo "	  mode: asyn/ack/syn"
	echo "	  name: message name"
	echo "	  body: message body, must json format"
	echo
}

#
#$1:mode
#$2:name
#$3:body...
#
main() {
	local argc=$#

	if ((argc<3)); then
		usage

		return ${e_inval}
	fi

	jmsg "${JSOCK_PEER}" "$@"
}

main "$@"
