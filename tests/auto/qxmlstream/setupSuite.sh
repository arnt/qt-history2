#!/bin/sh

cvs up ¥XML-Test-Suite

# We're working-around a bug in the test suite here.
sed -i 's+<TESTCASES PROFILE+<TESTCASES xml:base="eduni/namespaces/1.0/" PROFILE+' xmlconf/eduni/namespaces/1.0/rmt-ns10.xml
sed -i 's+<TESTCASES PROFILE+<TESTCASES xml:base="eduni/errata-2e/" PROFILE+' xmlconf/eduni/errata-2e/errata2e.xml

xmllint --valid --noent xmlconf/xmlconf.xml --output xmlconf/finalCatalog.xml


