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

#include "qmakeinterface.h"
#include <option.h>
#include <project.h>
#include <metamakefile.h>
#include <makefile.h>
#include <qdir.h>

QMakeInterface::QMakeInterface(const QString &file, int argc, char **argv) : profile(file), mkfile(0)
{
    Option::init(argc, argv);
}

QMakeInterface::QMakeInterface(bool findFile, int argc, char **argv) : mkfile(0)
{
    Option::init(argc, argv);
    if(!findFile)
        Option::mkfile::project_files.clear();
}

QMakeInterface::~QMakeInterface()
{
    
}

QStringList
QMakeInterface::buildStyles() const
{
    if(QMakeProject *p = getBuildStyle(QString()))
        return p->values("BUILDS");
    return QStringList();
}

QStringList
QMakeInterface::compileFlags(const QString &buildStyle, bool cplusplus) const
{
    if(QMakeProject *p = getBuildStyle(buildStyle))
        return p->values(cplusplus ? "QMAKE_CXXFLAGS" : "QMAKE_CFLAGS");
    return QStringList();
}

QStringList
QMakeInterface::defines(const QString &buildStyle) const
{
    if(QMakeProject *p = getBuildStyle(buildStyle))
        return p->values("PRL_EXPORT_DEFINES") + p->values("DEFINES");
    return QStringList();
}

QStringList
QMakeInterface::linkFlags(const QString &buildStyle) const
{
    if(QMakeProject *p = getBuildStyle(buildStyle))
        return p->values("QMAKE_LFLAGS");
    return QStringList();
}

QStringList
QMakeInterface::libraries(const QString &buildStyle) const
{
    if(QMakeProject *p = getBuildStyle(buildStyle)) {
        if(p->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
            return p->values("QMAKE_LIBS");
        QStringList ret;
        const QStringList &libs = p->values("QMAKE_INTERNAL_PRL_LIBS");
        for(int i = 0; i < libs.count(); i++) 
            ret += p->values(libs[i]);
        return ret;
    }
    return QStringList();
}

bool
QMakeInterface::addVariable(const QString &arg)
{
    Option::before_user_vars.append(arg);
    return true;
}

QMakeProject 
*QMakeInterface::getBuildStyle(const QString &buildStyle) const
{
    if(!mkfile) {
        QString oldpwd = QDir::currentPath();
        QString fn = profile;
        if(fn.isNull() && !Option::mkfile::project_files.isEmpty())
            fn = Option::mkfile::project_files.first();
        if(!fn.isEmpty()) {
            QFileInfo fileinfo(fn);
            if(fileinfo.isDir()) {
                if(!QDir::setCurrent(fileinfo.filePath()))
                    fprintf(stderr, "Cannot find directory: %s\n", 
                            fileinfo.filePath().latin1());
                fn = QString();
            } else if(fileinfo.path() != ".") {
                if(!QDir::setCurrent(fileinfo.path()))
                    fprintf(stderr, "Cannot find directory: %s\n", 
                            fileinfo.path().latin1());
                fn = fileinfo.filePath();
            }
        }
        QMakeProject *proj = new QMakeProject;
        uchar opts = QMakeProject::ReadAll;
        if(fn.isEmpty()) 
            opts ^= QMakeProject::ReadProFile;
        if(!proj->read(fn, opts)) {
            fprintf(stderr, "Failure to read: %s\n", fn.latin1());
            return 0;
        }
        Option::postProcessProject(proj);
        mkfile = MetaMakefileGenerator::createMakefileGenerator(proj);
        QDir::setCurrent(oldpwd);
    }
    Q_ASSERT(mkfile);
    if(buildStyle.isNull() || buildStyle == "default")
        return mkfile->projectFile();

    MakefileGenerator *ret = build_mkfile[buildStyle];
    if(!ret) {
        QMakeProject *base = mkfile->projectFile();
        //make sure it is specified
        bool found = false;
        const QStringList &builds = base->variables()["BUILDS"];
        for(int i = 0; i < builds.count(); i++) {
            if(builds.at(i) == buildStyle) {
                found = true;
                break;
            }
        }
        if(!found) {
            fprintf(stderr, "No buildStyle: %s\n", buildStyle.latin1());
            return 0;
        }
        //it is ugly how I just use this, but almost better than adding a weird parameter (IMHO)
        const QStringList old_user_config = Option::user_configs;
        if(!base->isEmpty(buildStyle + ".CONFIG"))
            Option::user_configs.prepend(base->values(buildStyle + ".CONFIG").join(" "));
        Option::user_configs.prepend(buildStyle);
        Option::user_configs.prepend("build_pass");
        QMakeProject *proj = new QMakeProject(base->properities());
        uchar opts = QMakeProject::ReadAll;
        if(base->projectFile().isEmpty()) 
            opts ^= QMakeProject::ReadProFile;
        proj->read(base->projectFile(), opts);
        Option::user_configs = old_user_config; 
        proj->variables()["CONFIG"] += "no_autoqmake";
        ret = MetaMakefileGenerator::createMakefileGenerator(proj);
        build_mkfile.insert(buildStyle, ret);
    }
    if(!ret)
        return 0;
    return ret->projectFile();
}
