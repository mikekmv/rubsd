#!/bin/sh

# $RuOBSD: beetraff-ipstatd.sh,v 1.3 2002/06/04 15:58:59 shadow Exp $

/usr/local/bin/dumpstat -h orion.oganer.net stat | /usr/local/bin/beecisco -u -n 217.196.96.114  -n 192.168.110.2
