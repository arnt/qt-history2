/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MINGW_MAKE_H
#define MINGW_MAKE_H

#include "winmakefile.h"

class MingwMakefileGenerator : public Win32MakefileGenerator
{
public:
    MingwMakefileGenerator();
    ~MingwMakefileGenerator();
private:
    bool isWindowsShell() const;
    void writeMingwParts(QTextStream &);
    void writeIncPart(QTextStream &t);
    void writeLibsPart(QTextStream &t);
    void writeLibDirPart(QTextStream &t);
    bool writeMakefile(QTextStream &);
    void writeObjectsPart(QTextStream &t);
    void writeBuildRulesPart(QTextStream &t);
    void writeRcFilePart(QTextStream &t);
    void init();
    void processPrlVariable(const QString &var, const QStringList &l);

    QStringList &findDependencies(const QString &file);
    
    QString preCompHeaderOut;

    virtual bool findLibraries();
    void fixTargetExt();

    bool init_flag;
    QString objectsLinkLine;
    QString quote;
};

inline MingwMakefileGenerator::~MingwMakefileGenerator()
{ }

#endif // MINGW_MAKE_H
