/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

#endif // PREPROCESSOR_H
