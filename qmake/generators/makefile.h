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
#ifndef __MAKEFILE_H__
#define __MAKEFILE_H__

#include "option.h"
#include "project.h"
#include "makefiledeps.h"
#include <qtextstream.h>
#include <qlist.h>

#ifdef Q_OS_WIN32
#define QT_POPEN _popen
#else
#define QT_POPEN popen
#endif

class MakefileGenerator : protected QMakeSourceFileInfo
{
    QString spec;
    bool init_opath_already, init_already, no_io;
    QStringList createObjectList(const QStringList &sources);
    QString build_args();
    void checkMultipleDefinition(const QString &, const QString &);
    QMap<QString, QString> fileFixed;
    QMap<QString, QMakeLocalFileName> depHeuristics;
    QMap<QString, QStringList> depends;

protected:
    void writePrlFile();
    void writeObj(QTextStream &, const QString &obj, const QString &src);
    void writeLexSrc(QTextStream &, const QString &lex);
    void writeYaccSrc(QTextStream &, const QString &yac);
    void writeInstalls(QTextStream &t, const QString &installs);

    void writeHeader(QTextStream &t);
    void writeSubDirs(QTextStream &t);
    void writeMakeQmake(QTextStream &t);
    void writeExtraVariables(QTextStream &t);
    void writeExtraTargets(QTextStream &t);
    void writeExtraCompilerTargets(QTextStream &t);
    void writeExtraCompilerVariables(QTextStream &t);

    bool verifyExtraCompiler(const QString &c, const QString &f);

    struct SubTarget
    {
        QString directory, profile, target, makefile;
    };
    enum SubTargetFlags {
        SubTargetsNoFlags=0x00,
        SubTargetInstalls=0x01,
        SubTargetOrdered=0x02
    };
    void writeSubTargets(QTextStream &t, QList<SubTarget*> subtargets, int flags);

    //interface to the source file info
    QMakeLocalFileName fixPathForFile(const QMakeLocalFileName &, bool);
    QMakeLocalFileName findFileForDep(const QMakeLocalFileName &, const QMakeLocalFileName &);
    QMakeProject *project;

    virtual void init();
    QString buildArgs();
    QString specdir();

    virtual QStringList &findDependencies(const QString &file);

    void setNoIO(bool o);
    bool noIO() const;

    virtual bool doDepends() const { return Option::mkfile::do_deps; }
    virtual bool writeMakefile(QTextStream &);
    void initOutPaths();

    //for cross-platform dependent directories
    virtual void usePlatformDir();

    //for installs
    virtual QString defaultInstall(const QString &);

    //for prl
    bool processPrlFile(QString &);
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual void processPrlFiles();
    virtual void writePrlFile(QTextStream &);

    //make sure libraries are found
    virtual bool findLibraries();

    virtual QString var(const QString &var);
    QString varGlue(const QString &var, const QString &before, const QString &glue, const QString &after);
    QString varList(const QString &var);
    QString val(const QStringList &varList);
    QString valGlue(const QStringList &varList, const QString &before, const QString &glue, const QString &after);
    QString valList(const QStringList &varList);

    virtual QString replaceExtraCompilerVariables(const QString &, const QString &, const QString &);

    QString filePrefixRoot(const QString &, const QString &);

    friend struct FileFixifyCacheKey;
    friend uint qHash(const FileFixifyCacheKey &f);
    enum FileFixifyType { FileFixifyAbsolute, FileFixifyRelative, FileFixifyDefault };
    QString fileFixify(const QString& file, const QString &out_dir=QString::null,
                       const QString &in_dir=QString::null, FileFixifyType fix=FileFixifyDefault, bool canon=true) const;
    inline QString fileFixify(const QString& file, FileFixifyType fix, bool canon=true) const { return fileFixify(file, QString::null, QString::null, fix, canon); }
    QStringList fileFixify(const QStringList& files, const QString &out_dir=QString::null,
                           const QString &in_dir=QString::null, FileFixifyType fix=FileFixifyDefault, bool canon=true) const;
public:
    MakefileGenerator();
    virtual ~MakefileGenerator();
    QMakeProject *projectFile() const;
    void setProjectFile(QMakeProject *p);

    static MakefileGenerator *create(QMakeProject *);
    virtual bool write();
    virtual bool writeProjectMakefile();
    virtual bool supportsMetaBuild() { return true; }
    virtual bool supportsMergedBuilds() { return false; }
    virtual bool mergeBuildProject(MakefileGenerator * /*other*/) { return false; }
    virtual bool openOutput(QFile &, const QString &build) const;
};

inline void MakefileGenerator::setNoIO(bool o)
{ no_io = o; }

inline bool MakefileGenerator::noIO() const
{ return no_io; }

inline QString MakefileGenerator::defaultInstall(const QString &)
{ return QString(""); }

inline bool MakefileGenerator::findLibraries()
{ return true; }

inline MakefileGenerator::~MakefileGenerator()
{ }

QString mkdir_p_asstring(const QString &dir);

#endif /* __MAKEFILE_H__ */
