/****************************************************************************
**
** Definition of MingwMakefileGenerator class.
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

#ifndef __MINGW_MAKE_H__
#define __MINGW_MAKE_H__

#include "winmakefile.h"

class MingwMakefileGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    void writeMingwParts(QTextStream &);
    void writeSubDirs(QTextStream &t) ;

    bool writeMakefile(QTextStream &);
    void init();
    
    virtual bool findLibraries();
    void processLibsVar();
    void fixTargetExt();

public:
    MingwMakefileGenerator(QMakeProject *p);
    ~MingwMakefileGenerator();

};

inline MingwMakefileGenerator::~MingwMakefileGenerator()
{ }

#endif /* __MINGW_MAKE_H__ */
