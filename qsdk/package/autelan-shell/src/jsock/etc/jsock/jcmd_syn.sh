#!/bin/bash

. ${__ROOTFS__}/etc/jsock/jsock.in


usage() {
	echo "$0 usage:"
	echo "  $0 {name} [args...]"
	echo "	  name: remote command name"
	echo "	  args: remote command args"
	echo
}

#
#$1:name
#[$2:args...]
#
main() {
	local argc=$#

	if ((argc<1)); then
		usage

		return ${e_inval}
	fi

	jcmd "${JSOCK_PEER}" "syn" "$@"
}

main "$@"
