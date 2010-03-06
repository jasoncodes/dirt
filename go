#!/bin/bash -e

BASEDIR="`dirname "$0"`"

chmod +x "$BASEDIR"/{go,run}

ANT_PARAMS=""
RUN_PARAMS=""
for PARAM in "$@"
do
	if echo $PARAM | grep "^--" > /dev/null
	then
		RUN_PARAMS="$RUN_PARAMS $PARAM"
	else
		ANT_PARAMS="$ANT_PARAMS $PARAM"
	fi
done

ant $ANT_PARAMS
"$BASEDIR"/run $RUN_PARAMS
