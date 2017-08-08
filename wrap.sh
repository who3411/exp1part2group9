#!/bin/bash

declare -r MAXCONNECT=1024
declare -r MAXPROCESS=8

declare -i i
declare -i j

if [ $# -ne 2 ]; then
	echo "usage: ${0} [ip address] [port]"
	exit 1
fi

echo -n "set your output file    -> "
read OUTFILE

if [ -e ${OUTFILE} ]; then
	echo -n "${OUTFILE} exist, so overwrite ${OUTFILE}. OK?[y/n]"
	read flag

	if [ "${flag}" = "y" ] || [ "${flag}" = "Y" ]; then
		echo "OK, ${OUTFILE} overrwite."
	elif [ "${flag}" = "n" ] || [ "${flag}" = "N" ]; then
		echo "This program exit. restart this program."
		exit 0
	else
		echo "Sorry, cannot understand."
		exit 1
	fi
fi

echo -n "set your measure method -> "
read METHOD

i=1
while [ ${i} -le ${MAXPROCESS} ]; do
	j=`expr ${MAXPROCESS} / ${i}`
	connectcount=0
	connectcount=`expr ${i} \* ${j}`
	while [[ ${connectcount} -le ${MAXCONNECT} ]]; do
		sleep 5s
		echo "process: ${i}, thread: ${j} start!"
		./mymeasure.sh ${1} ${j} ${i} ${2} ${OUTFILE} ${METHOD}
		echo "process: ${i}, thread: ${j} end!"
		j=`expr ${j} \* 2`
		connectcount=`expr ${i} \* ${j}`
	done
	i=`expr ${i} \* 2`
done
