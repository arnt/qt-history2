#!/bin/sh
# 
# Hello!
#
# You don't have to run this script unless you are actually updating the test suite.
# For precaution, we therefore have this exit call.


# CVS is retarded when it comes to reverting changes. Remove files it has moved.
find XML-Test-Suite/ -name ".*.?.*" | xargs rm

cd XML-Test-Suite

export CVSROOT=":pserver:anonymous@dev.w3.org:/sources/public"
cvs -q up -C

# We're working-around a bug in the test suite here.
#sed -i 's+<TESTCASES PROFILE+<TESTCASES xml:base="eduni/namespaces/1.0/" PROFILE+' xmlconf/eduni/namespaces/1.0/rmt-ns10.xml
#sed -i 's+<TESTCASES PROFILE+<TESTCASES xml:base="eduni/errata-2e/" PROFILE+' xmlconf/eduni/errata-2e/errata2e.xml

sed -i 's+XML1.0-errata2e+XML1.0-errata2e|XML1.0-errata3e|NS1.0-errata1e+' xmlconf/testcases.dtd

#sed -i 's+NS1.0-errata1+XML1.0-errata2e+' xmlconf/eduni/namespaces/errata-1e/errata1e.xml

p4 edit ...
p4 revert `find -name "Entries"` # They only contain CVS timestamps.
xmllint --valid --noent xmlconf/xmlconf.xml --output xmlconf/finalCatalog.xml
p4 revert -a ...
