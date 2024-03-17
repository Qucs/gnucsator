#!/bin/bash
#
# Qucs compatibility script. Processes args like Qucsator and run Gnucsator.
# Also, collate output files into .dat understood by (legacy) Qucs.

TEMP=`getopt -o i:o:a:bg --long input:output: \
     -n 'gnucsator.sh' -- "$@"`

if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi
eval set -- "$TEMP"
ARGS=""

while true; do
	case "$1" in
		-b) shift;;
		-g) shift;;
		-i|--input) infile="$2"; shift 2;;
		-o|--output) outfile="$2"; shift 2;;
		-a) ARGS="${ARGS} -a $2"; shift 2;;
		--) shift; break ;;
		*) echo "Internal error!" ; exit 1 ;;
	esac
done

if [ -z "$GNUCSATOR" ]; then
GNUCSATOR=gnucsator
fi

if [ -z "$outfile" ]; then
	outfile=outfile.dat
fi

if [ -z "$infile" ]; then
	echo no input file specified.
	exit 1
fi

b=$( basename ${infile} )
out=${b}.tmp

echo "<Qucs Dataset 0.0.19>" > $outfile

rm -f $out.dc
rm -f $out.tr
rm -f $out.sp
rm -f $out.ac

$GNUCSATOR $ARGS <<EOF
qucs
include $infile
go ${out}
status notime
EOF

[ -f $out.dc ] && cat $out.dc >> $outfile
[ -f $out.tr ] && cat $out.tr >> $outfile
[ -f $out.sp ] && cat $out.sp >> $outfile
[ -f $out.ac ] && cat $out.ac >> $outfile

:
