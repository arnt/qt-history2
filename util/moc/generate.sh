#!/bin/sh
p4 edit ../../src/tools/moc/keywords.cpp
p4 edit ../../src/tools/moc/ppkeywords.cpp
make
./generate_keywords > ../../src/tools/moc/keywords.cpp
./generate_keywords preprocessor > ../../src/tools/moc/ppkeywords.cpp
p4 revert -a ../../src/tools/moc/keywords.cpp
p4 revert -a ../../src/tools/moc/ppkeywords.cpp
