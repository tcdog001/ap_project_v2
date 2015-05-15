#!/bin/bash
main() {
    udhcpc -i eth1
    /etc/jsock/jcmd.sh syn "stop_3g"
    /etc/jsock/jcmd.sh syn "route add default gw 1.0.0.1" 2>/dev/null
}

main "$@"
