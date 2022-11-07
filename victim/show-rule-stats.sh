#!/usr/bin/env bash
# -*- mode: sh; coding: utf-8; fill-column: 80; -*-
#
# Show matches against rules.
#

readonly CMD="sudo iptables -L -n -v -x"

echo vagrant ssh -c "'$CMD'" | bash
