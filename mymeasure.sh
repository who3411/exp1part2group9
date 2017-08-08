#!/bin/bash

declare -r MEASURETIMES=5
declare -r RESULTNUM=4

declare    retstr
declare -i i
declare -i j

if [ $# -ne 6 ]; then
	echo "usage: ${0} [ip address] [# of thread] [# of process] [port] [outfile] [method]"
	exit 1
fi

echo "outfile = ${5}, method = ${6}"

for i in `seq 0 $(expr ${MEASURETIMES} - 1)`; do
	sleep 2s
	echo "$(expr ${i} + 1)/${MEASURETIMES} start!"
	echo "${1} ${2} ${3} ${4}"
	tmpresult=`./HTTPBench.out ${1} ${2} ${3} ${4}`
	echo "$(expr ${i} + 1)/${MEASURETIMES} end!"
	j=0
	onestr=""
	while read val; do
		tmpval=`echo -n "${val}" | perl -pe "s/.*(:| is )([0-9]+\.[0-9]+)/\2/g"`
		onestr=$onestr$tmpval
		j=`expr ${j} + 1`
		if [ ${j} -ne ${RESULTNUM} ]; then
			onestr="${onestr} "
		fi
	done <<< "${tmpresult}"
	eval 'tmparray=(${onestr})'

	for j in `seq 0 $(expr ${RESULTNUM} - 1)`; do
		eval "result${i}${j}=\${tmparray[${j}]}"
		#eval echo -n "\${result${i}${j}},"
	done
	#echo
done

retstr="${6},$(expr ${2} \* ${3}),${2},${3},"
for j in `seq 0 $(expr ${RESULTNUM} - 1)`; do
	for i in `seq 0 $(expr ${MEASURETIMES} - 1)`; do
		#echo "i=${i}"
		eval "retstr=\$retstr\$result${i}${j}"
		retstr="${retstr},"
	done
	retstr="$retstr,,"
done
if ! [ -e ${5} ]; then
	myhead="method,request,thread,process,"
	myhead="${myhead}tcp(1),tcp(2),tcp(3),tcp(4),tcp(5),,,"
	myhead="${myhead}http(1),http(2),http(3),http(4),http(5),,,"
	myhead="${myhead}total(1),total(2),total(3),total(4),total(5),,,"
	myhead="${myhead}error(1),error(2),error(3),error(4),error(5),,,"
	echo "${myhead}" > ${5}
fi
echo ${retstr} >> ${5}
