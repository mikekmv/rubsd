#!/bin/sh

# $RuOBSD: beetraff-ipstatd.sh,v 1.1 2002/04/02 05:18:00 tm Exp $

/usr/local/bin/dumpstat -h vega.oganer.net stat | /usr/local/bin/beecisco -u -n 192.168.112.250 -n 195.161.62.202
