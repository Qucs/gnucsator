#!/bin/bash
# taken out of gnucsator.sh

# this is stupid (makes gnucap appear slow), but this must be fixed in qucs.
# see simulator output PRs in qucs.
postproc()
{
	mode=wait
	n=0
	m=0
	while read line; do
		if [ $mode = tran ]; then
			# below.
			:
		elif [ $mode = verbose ]; then
			if [ "$line" = @@@ENDVERBOSE@@@ ]; then
				mode=wait
				continue
			fi
			echo $line
			continue
		elif [ "${line:0:5}"x = "#Timex" ]; then
			probes=( ${line:5:${#line}} )
			mode=tran;
			continue
		elif [ $mode == wait ]; then
			if [ "$line" = @@@VERBOSE@@@ ]; then
				mode=verbose
			fi
			continue
		fi


		[ -z "$line" ] && break;
		if [ "${line:0:1}" = "#" ]; then
			mode=wait
			continue
		elif [ "${line:0:5}"x = "===DCx" ]; then
			mode=dc
			continue
		fi
		m=0
		for tmp in $line; do
			eval "data_${n}_$m=\"$tmp\""
			m=`expr $m + 1`
		done
		n=`expr $n + 1`
	done
	m=`expr $m - 1`
	echo "<indep time $n>"
	n=`expr $n - 1`
	for i in `seq 0 $n`; do
		eval "echo \ \ +\$data_${i}_0"
	done
	echo "</indep>"
	for i in `seq 1 $m`; do
		tmp="${probes[`expr $i - 1`]}"
		br=${tmp##*(}
		what=${tmp%%(*}
		echo "<dep ${br:0:-1}.${what^}t time>"
		for j in `seq 0 $n`; do
			eval "data=\$data_${j}_$i"
			echo -n "  "
			[ "${data:0:1}" = "-" ] || echo -n +
			echo "$data"
		done
		echo "</dep>"
	done
}

postproc
