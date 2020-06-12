#!/bin/sh

FRANNY="/usr/local/bin/franny"

insert_dir()
{
	for FILE in `ls $2`; do
		if [ -d $2/$FILE ]; then
			$FRANNY -M $3$FILE $1
			insert_dir $1 $2/$FILE $3$FILE/
		else
			echo "Processing file $3$FILE"
			IN_FILE=$2/$FILE
			OUT_FILE=$3$FILE
			$FRANNY -A -i $IN_FILE -o $OUT_FILE $1
		fi
	done
}

if [ "$1" = "" ] || [ "$2" = "" ]; then
	echo "Usage: $0 file.atr source_directory"
	exit
fi
insert_dir $1 $2 ""
