/****************************************************************************
**
** Definition of MingwMakefileGenerator class.
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

#ifndef __MINGW_MAKE_H__
#define __MINGW_MAKE_H__

#include "winmakefile.h"

class MingwMakefileGenerator : public Win32MakefileGenerator
{
public:
    MingwMakefileGenerator(QMakeProject *p);
    ~MingwMakefileGenerator();
private:
    void writeMingwParts(QTextStream &);
    void writeSubDirs(QTextStream &t);
    void writeLibsPart(QTextStream &t);
    bool writeMakefile(QTextStream &);
    void writeObjectsPart(QTextStream &t);
    void writeObjMocPart(QTextStream &t);
    void writeBuildRulesPart(QTextStream &t, const QString &extraCompilerDeps);
    void writeRcFilePart(QTextStream &t);
    void init();

    virtual bool findLibraries();
    void processLibsVar();
    void fixTargetExt();
    void processQtConfig();

    bool init_flag;
    QString objmocLinkLine, objectsLinkLine;
};

inline MingwMakefileGenerator::~MingwMakefileGenerator()
{ }

#endif /* __MINGW_MAKE_H__ */
