#!/bin/bash
main() {
    /etc/init.d/network restart
    /etc/jsock/jcmd.sh syn "route del default gw 1.0.0.1" 2>/dev/null
    /etc/jsock/jcmd.sh syn "start_3g" &
}

main "$@"
