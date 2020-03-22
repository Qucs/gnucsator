#!/bin/bash
# this is gnucsator, gnucap in qucsator mode.
# it loads plugins and switches to qucs language.
#
# this sucks.
# to be replaced by a "qucsator" binary, once
# shared library and output pluggability are ready.
# the .sh extension is intentional.

TEMP=`getopt -o i:o:bg --long input:output: \
     -n 'gnucsator.sh' -- "$@"`

if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi
eval set -- "$TEMP"

while true; do
	case "$1" in
		-b) shift;;
		-g) shift;;
		-i|--input) infile="$2"; shift 2;;
		-o|--output) outfile="$2"; shift 2;;
		--) shift; break ;;
		*) echo "Internal error!" ; exit 1 ;;
	esac
done

GNUCSATOR=gnucsator

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

if [ -z "$outfile" ]; then
	outfile=outfile.dat
fi

out=${infile}.tmp

echo "<Qucs Dataset 0.0.19>" > $outfile

postproc_tr=${GNUCSATOR_POSTOROC_TR-postproc}

rm -f $out.dc
rm -f $out.tr
rm -f $out.sp
rm -f $out.ac

$GNUCSATOR <<EOF
qucs
include $infile
go ${out}
status notime
EOF

[ -f $out.dc ] && cat $out.dc >> $outfile
[ -f $out.tr ] && cat $out.tr | $postproc_tr >> $outfile
[ -f $out.sp ] && cat $out.sp >> $outfile
[ -f $out.ac ] && cat $out.ac >> $outfile

:
