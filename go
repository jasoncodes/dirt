#!/bin/bash -e
ant
if [ "`uname -s`" == "Darwin" ]
then
	dist/Dirt.app/Contents/MacOS/JavaApplicationStub
else
	java -jar dist/Dirt.jar
fi
