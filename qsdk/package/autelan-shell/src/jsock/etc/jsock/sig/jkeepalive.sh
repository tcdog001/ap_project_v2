#!/bin/bash

. ${__ROOTFS__}/etc/jsock/sig/sig.in

#
# NOT used now!!!
#
main() {
	local seq=$(getfilevalue ${file_keepalive_seq} 0)
	local state=$(getfilevalue ${file_keepalive_state} 0)
	local count=$(getfilevalue ${file_keepalive_count} 0)
	local threshold=$(getfilevalue ${file_keepalive_threshold} ${keepalive_threshold})
	local interval=${keepalive_interval}

	((count++))
	if ((count>threshold)); then
		do_nithing
	fi
}

main "$@"
