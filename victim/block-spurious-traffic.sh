#!/usr/bin/env bash
# -*- mode: sh; coding: utf-8; fill-column: 80; -*-
#
# Block spurious traffic.
#


readonly SRC_IP="192.168.10.30"
readonly DST_IP="192.168.10.10"
readonly RULE="-p udp -s $SRC_IP -d $DST_IP --destination-port 53"
readonly CMD="sudo iptables -I INPUT $RULE -j DROP"

echo vagrant ssh -c "'$CMD'" | bash
