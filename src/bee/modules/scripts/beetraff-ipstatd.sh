#!/bin/sh

# $RuOBSD$

dumpstat -h vega.oganer.net stat | /usr/local/bin/beecisco -u -n 192.168.112.250 -n 195.161.62.202
