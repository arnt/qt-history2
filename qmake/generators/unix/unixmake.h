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

#ifndef __UNIXMAKE_H__
#define __UNIXMAKE_H__

#include "makefile.h"

class UnixMakefileGenerator : public MakefileGenerator
{
    bool init_flag, include_deps;
    bool writeMakefile(QTextStream &);
    QString libtoolFileName();
    void writeLibtoolFile();     // for libtool
    QString pkgConfigPrefix() const;
    QString pkgConfigFileName();
    QString pkgConfigFixPath(QString) const;
    void writePkgConfigFile();   // for pkg-config
    void writePrlFile(QTextStream &);

public:
    UnixMakefileGenerator();
    ~UnixMakefileGenerator();

protected:
    virtual bool doPrecompiledHeaders() const { return project->isActiveConfig("precompile_header"); }
    virtual bool doDepends() const { return !include_deps && MakefileGenerator::doDepends(); }
    virtual QString defaultInstall(const QString &);
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual void processPrlFiles();

    virtual bool findLibraries();
    virtual QStringList &findDependencies(const QString &);
    virtual void init();

    void writeMakeParts(QTextStream &);

private:
    void init2();
};

inline UnixMakefileGenerator::~UnixMakefileGenerator()
{ }


#endif /* __UNIXMAKE_H__ */
