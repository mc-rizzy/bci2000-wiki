#! /usr/bin/env bash

ver=3.6
rev=$(svn info | grep Revision)
rev=$(IFS=" " ; set -- $rev ; echo $2)
echo "$ver.$rev"

