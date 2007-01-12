#!/bin/sh

${FLEX-flex} -oglsl-lex.incl glsl-lex.l
${QLALR-qlalr} --debug glsl.g

qmake
make
