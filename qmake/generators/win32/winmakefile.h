/****************************************************************************
**
** Definition of Win32MakefileGenerator class.
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
    virtual void writeSubDirs(QTextStream &t);
    virtual void writeExtraCompilerParts(QTextStream &t);
    virtual void writeExtraTargetParts(QTextStream &t);
    virtual void writeCleanParts(QTextStream &t);
    int findHighestVersion(const QString &dir, const QString &stem);
    bool findLibraries(const QString &);
    QString findDependency(const QString &);
    virtual bool findLibraries();
    virtual void processPrlFiles();
    virtual void processVars();
    virtual void processLibsVar();
    virtual void fixTargetExt();
    virtual void processRttiConfig();
    virtual void processMocConfig();
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
