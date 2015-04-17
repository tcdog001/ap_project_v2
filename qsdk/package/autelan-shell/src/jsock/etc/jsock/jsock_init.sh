#!/bin/bash

. ${__ROOTFS__}/etc/jsock/jsock.in

main() {
	case "$(get_board_type)" in
	"md")
		xinetd &
		;;
	"ap")
		do_nothing
		;;
	*)
		do_nothing
		;;
	esac

	${__ROOTFS__}/etc/jsock/sig/keepalive.sh &
}

main "$@"
