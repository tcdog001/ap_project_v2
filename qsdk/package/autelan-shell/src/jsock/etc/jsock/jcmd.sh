#!/bin/bash

. ${__ROOTFS__}/etc/jsock/jsock.in


usage() {
	echo "$0 usage:"
	echo "  $0 {mode} {name} [args...]"
	echo "	  mode: asyn/ack/syn"
	echo "	  name: remote command name"
	echo "	  args: remote command args"
	echo
}

#
#$1:mode
#$2:name
#[$3:args...]
#
main() {
	local argc=$#

	if ((argc<2)); then
		usage

		return ${e_inval}
	fi

	jcmd "${JSOCK_PEER}" "$@"
}

main "$@"
