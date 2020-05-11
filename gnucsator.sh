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

# translating and ignoring qucsator arguments to make a gnucsator call
while true; do
	case "$1" in
		-b) shift;; #-b, --bar      enable textual progress bar
		-g) shift;; #-g, --gui      special progress bar used by gui
		-i|--input) infile="$2"; shift 2;;
		-o|--output) outfile="$2"; shift 2;;
		--) shift; break ;;
		*) echo "Internal error!" ; exit 1 ;;
	esac
done

GNUCSATOR=gnucsator

if [ -z "$outfile" ]; then
	outfile=outfile.dat
fi

out=${infile}.tmp

# alternatives to translate2qucs: qucs_data_convert.py, postproc.sh
postproc_tr=${GNUCSATOR_POSTPROC_TR:-translate2qucs}

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

echo "<Qucs Dataset>"   > $outfile
[ -f $out.dc ] && cat $out.dc >> $outfile
[ -f $out.tr ] && cat $out.tr | $postproc_tr >> $outfile
[ -f $out.sp ] && cat $out.sp >> $outfile
[ -f $out.ac ] && cat $out.ac >> $outfile

:
