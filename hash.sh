#!/bin/sh
# script to determine git hash of current source tree
# try to use whatever git tells us if there is a .git folder
if [ -d .git -a -r .git ]
then
	hash=$(git log 2>/dev/null | head -n1 2>/dev/null | sed "s/.* //" 2>/dev/null)
fi

if [ x"$hash" != x ]
then
	echo $hash
else
	echo "UNKNOWN"
fi
exit 0
