/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BORLAND_BMAKE_H
#define BORLAND_BMAKE_H

#include "winmakefile.h"

QT_BEGIN_NAMESPACE

class BorlandMakefileGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    void writeBorlandParts(QTextStream &);
    void writeBuildRulesPart(QTextStream &t);
    void writeCleanParts(QTextStream &t);
    bool writeMakefile(QTextStream &);
    void init();

public:
    BorlandMakefileGenerator();
    ~BorlandMakefileGenerator();
};

inline BorlandMakefileGenerator::~BorlandMakefileGenerator()
{ }

QT_END_NAMESPACE

#endif // BORLAND_BMAKE_H
