#!/bin/sh

# $RuOBSD: beetraff-ipstatd.sh,v 1.2 2002/06/01 19:16:50 shadow Exp $

/usr/local/bin/dumpstat -h orion.oganer.net stat | /usr/local/bin/beecisco -u -n 192.168.119.250 -n 217.196.96.114
