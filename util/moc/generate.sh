#!/bin/sh
p4 edit ../../src/moc/keywords.cpp
p4 edit ../../src/moc/ppkeywords.cpp
make
./generate_keywords > ../../src/moc/keywords.cpp
./generate_keywords preprocessor > ../../src/moc/ppkeywords.cpp
p4 revert -a ../../src/moc/keywords.cpp
p4 revert -a ../../src/moc/ppkeywords.cpp
