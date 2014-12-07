#!/bin/bash

PATH=$HOME/bin

function usage()
{
	echo ""
	echo "hexdiff: by default, add second provided hex parameter to first."
	echo "If the first argument is '-', subtract instead."
	echo "Please supply two arguments that begin with '0x'."
	echo ""
	exit 1
}

if ( [[ $# -ne 2 ]] && [[ $# -ne 3 ]] ) ; then
   usage
   exit 1
fi

if [[ $# -eq 3 ]] ; then
   if [[ ! $1 =~ "-" ]]  ; then
      usage
      exit 1
   else
      first=`$HOME/bin/hex2dec $2`
      if [ $? -gt 0 ] ; then
            usage
	    exit 1
      fi

      second=`$HOME/bin/hex2dec $3`
      if [ $? -gt 0 ] ; then
            usage
	    exit 1
      fi

      result=$(($first - $second))
   fi
else
      first=`$HOME/bin/hex2dec $1`
      if [ $? -gt 0 ] ; then
            usage
	    exit 1
      fi

      second=`$HOME/bin/hex2dec $2`
      if [ $? -gt 0 ] ; then
         usage
	 exit 1
      fi

      result=$(($first + $second))
fi

echo `$HOME/bin/dec2hex $result`
