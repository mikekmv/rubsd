#!/bin/sh

# $RuOBSD$

ipfstat -aio | /usr/local/bin/beetraff -u
