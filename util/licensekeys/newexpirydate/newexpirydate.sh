#!/bin/sh

NewExpiryDate=2037-12-31

if test -z "$1"; then
    echo "usage: $0 license-file"
    exit 1
fi

if test -f "$1"; then
    . $1
else
    echo "$1 does not exist"
    exit 1
fi


cmdline=`../decode/decode $LicenseKeyExt | sed -e "s,^,\",g" -e "s,$,\",g" | head -n 5`
NewLicenseKeyExt=`eval ../generate/generate $cmdline $NewExpiryDate`

sed -e "s,^ExpiryDate=.*$,ExpiryDate=$NewExpiryDate," \
    -e "s,^LicenseKeyExt=.*$,LicenseKeyExt=$NewLicenseKeyExt," \
    $1 > x
mv x $1
