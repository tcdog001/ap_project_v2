#!/bin/bash

. ${__ROOTFS__}/etc/jsock/sig/jkeepalive.in

main() {
	keepalive_send
}

main "$@"
