#!/usr/bin/env bash
# -*- mode: sh; coding: utf-8; fill-column: 80; -*-
#
# Verify whether the DNS 'A' record for a given domain has indeed been spoofed.
#

readonly NS_IP='192.168.10.10'

function show_usage {
    echo "Usage: ${0} <domain>" >& 2
}

[ $# -ne 1 ] && show_usage && exit 1

readonly DOM="$1"
readonly CMD="dig @$NS_IP +short $DOM"
echo vagrant ssh -c "'$CMD'"
