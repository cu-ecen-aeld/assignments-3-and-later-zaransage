#!/bin/bash

filesdir=$1
searchstr=$2


if [ -z ${filesdir} ]
then
echo "You have not specified 'filesdir' argument"
exit 1
fi

if [ ! -d ${filesdir} ]
then
echo "This directory does not exist"
exit 1
fi

if [ -z ${searchstr} ]
then
echo "You have not specified 'searchstr' argument"
exit 1
fi

