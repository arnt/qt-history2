/****************************************************************************
**
** Definition of Win32MakefileGenerator class.
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

#ifndef __WINMAKEFILE_H__
#define __WINMAKEFILE_H__

#include "makefile.h"

// In the Qt evaluation and educational version, we have a postfix in the
// library name (e.g. qtmteval301.dll). QTDLL_POSTFIX is used for this.
// A script modifies these lines when building eval/edu version, so be careful
// when changing them.
#ifndef QTDLL_POSTFIX
#define QTDLL_POSTFIX ""
#endif

class Win32MakefileGenerator : public MakefileGenerator
{
public:
    Win32MakefileGenerator(QMakeProject *p);
    ~Win32MakefileGenerator();
protected:
    virtual QString defaultInstall(const QString &);
    virtual void writeSubDirs(QTextStream &t);
    virtual QString writeObjCompParts(QTextStream &t);
    virtual void writeCleanParts(QTextStream &t);
    virtual void writeStandardParts(QTextStream &t);
    virtual void writeLibDirPart(QTextStream &t);
    virtual void writeLibsPart(QTextStream &t);
    virtual void writeObjectsPart(QTextStream &t);
    virtual void writeObjMocPart(QTextStream &t);
    virtual void writeImplicitRulesPart(QTextStream &t);
    virtual void writeBuildRulesPart(QTextStream &, const QString &);
    virtual void writeRcFilePart(QTextStream &t);

    int findHighestVersion(const QString &dir, const QString &stem);
    bool findLibraries(const QString &);
    QString findDependency(const QString &);
    virtual bool findLibraries();

    virtual void processMocConfig();
    virtual void processPrlFiles();
    virtual void processVars();
    virtual void processLibsVar();
    virtual void fixTargetExt();
    virtual void processRcFileVar();
    virtual void processExtraWinCompilersVar();
    virtual void processQtConfig();
    virtual void processDllConfig();
    virtual void processFileTagsVar();
};

inline Win32MakefileGenerator::~Win32MakefileGenerator()
{ }

inline bool Win32MakefileGenerator::findLibraries()
{ return findLibraries("QMAKE_LIBS"); }



#endif /* __WINMAKEFILE_H__ */
