/****************************************************************************
**
** Definition of NmakeMakefileGenerator class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of qmake.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __MSVC_NMAKE_H__
#define __MSVC_NMAKE_H__

#include "winmakefile.h"

class NmakeMakefileGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    void writeNmakeParts(QTextStream &);
    QStringList &findDependencies(const QString &file);

    bool writeMakefile(QTextStream &);
    void init();

public:
    NmakeMakefileGenerator(QMakeProject *p);
    ~NmakeMakefileGenerator();

};

inline NmakeMakefileGenerator::~NmakeMakefileGenerator()
{ }

#endif /* __MSVC_NMAKE_H__ */
