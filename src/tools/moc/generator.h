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

#ifndef GENERATOR_H
#define GENERATOR_H

#include "moc.h"

class Generator
{
    FILE *out;
    ClassDef *cdef;
public:
    Generator(FILE *outfile, ClassDef *classDef);
    void generateCode();
private:
    void generateClassInfos();
    void generateFunctions(QList<FunctionDef> &list, const char *functype, int type);
    void generateEnums(int index);
    void generateProperties();
    void generateMetacall();
    void generateSignal(FunctionDef *def, int index);

    int strreg(const char *); // registers a string and returns its id
    QList<QByteArray>strings;
    QByteArray purestSuperClass;
};

#endif // GENERATOR_H
