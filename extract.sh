#!/bin/sh

FRANNY="/usr/local/bin/franny"
DIR_FORMAT="A!F+.E"

extract_dir()
{
	for ROW in `$FRANNY -L"$3*.*" -l $DIR_FORMAT $1 | grep !`; do
		FILE="`echo $ROW | cut -d! -f 2 -s`"
		DIR="`echo $ROW | cut -c 3`"
		if [ "$DIR" = "D" ]; then
			mkdir -p $2/$FILE
			extract_dir $1 $2/$FILE $3$FILE/
		else
			echo "Processing file $3$FILE"
			IN_FILE="$3$FILE"
			OUT_FILE="$2/$FILE"
			$FRANNY -i $IN_FILE -o $OUT_FILE -S $1
		fi
	done
}

if [ "$1" = "" ] || [ "$2" = "" ]; then
	echo "Usage: $0 file.atr target_directory"
	exit
fi
mkdir -p $2
extract_dir $1 $2 ""
