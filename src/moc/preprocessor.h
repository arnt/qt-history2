#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "symbols.h"
#include <qlist.h>
#include <qmap.h>
#include <stdio.h>

typedef QMap<QByteArray,QByteArray> Macros;

class Preprocessor
{
public:
    static bool onlyPreprocess;
    static QByteArray protocol;
    static QList<QByteArray> includes;
    static Macros macros;
    static QByteArray preprocessed(const QByteArray &filename, FILE *file);
};

#endif
