#!/bin/bash
main() {
	cp /etc/config/network.dev /etc/config/network
	/etc/init.d/network reload
	/etc/jsock/jcmd.sh syn "stop_3g"
	/etc/jsock/jcmd.sh syn "route del default ppp0" 2>/dev/null
	/etc/jsock/jcmd.sh syn "route add default gw 1.0.0.1" 2>/dev/null
}

main "$@"
