/****************************************************************************
**
** Definition of BorlandMakefileGenerator class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of qmake.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __BORLAND_BMAKE_H__
#define __BORLAND_BMAKE_H__

#include "winmakefile.h"

class BorlandMakefileGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    void writeBorlandParts(QTextStream &);
    void writeBuildRulesPart(QTextStream &t);
    void writeCleanParts(QTextStream &t);
    bool writeMakefile(QTextStream &);
    void init();

public:
    BorlandMakefileGenerator(QMakeProject *p);
    ~BorlandMakefileGenerator();
};

inline BorlandMakefileGenerator::~BorlandMakefileGenerator()
{ }

#endif /* __BORLAND_BMAKE_H__ */
