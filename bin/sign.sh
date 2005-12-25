#!/bin/bash
stty -echo
echo -n "Enter passphrase: "
read KEY
stty echo
echo
for FILENAME in dist/webstart/*.jar dist/webstart/mac/*.jar
do
	jarsigner -storetype pkcs12 -keystore ~/Documents/jason\@gslabs.com.au.p12 -storepass "$KEY" "$FILENAME"  "thawte freemail member's thawte consulting (pty) ltd. id" || exit 1
done
unset KEY
