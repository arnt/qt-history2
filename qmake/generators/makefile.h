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
#include <qhash.h>
#include <qfileinfo.h>

#ifdef Q_OS_WIN32
#define QT_POPEN _popen
#else
#define QT_POPEN popen
#endif

class MakefileGenerator : protected QMakeSourceFileInfo
{
    QString spec;
    bool init_opath_already, init_already, no_io;
    QHash<QString, bool> init_compiler_already;
    QStringList createObjectList(const QStringList &sources);
    QString build_args();
    void checkMultipleDefinition(const QString &, const QString &);

    //internal caches
    mutable QHash<QString, QMakeLocalFileName> depHeuristicsCache;
    mutable QHash<QString, QStringList> dependsCache;

protected:
    //makefile style generator functions
    void writeObj(QTextStream &, const QString &src);
    void writeLexSrc(QTextStream &, const QString &lex);
    void writeYaccSrc(QTextStream &, const QString &yac);
    void writeInstalls(QTextStream &t, const QString &installs, bool noBuild=false);
    void writeHeader(QTextStream &t);
    void writeSubDirs(QTextStream &t);
    void writeMakeQmake(QTextStream &t);
    void writeExtraVariables(QTextStream &t);
    void writeExtraTargets(QTextStream &t);
    void writeExtraCompilerTargets(QTextStream &t);
    void writeExtraCompilerVariables(QTextStream &t);
    virtual bool writeMakefile(QTextStream &);

    //generating subtarget makefiles
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

    //extra compiler interface
    bool verifyExtraCompiler(const QString &c, const QString &f);
    virtual QString replaceExtraCompilerVariables(const QString &, const QString &, const QString &);

    //interface to the source file info
    QMakeLocalFileName fixPathForFile(const QMakeLocalFileName &, bool);
    QMakeLocalFileName findFileForDep(const QMakeLocalFileName &, const QMakeLocalFileName &);
    QFileInfo findFileInfo(const QMakeLocalFileName &);
    QMakeProject *project;

    //initialization
    virtual void init();
    void initOutPaths();
    struct Compiler
    {
        QString variable_in;
        enum CompilerFlags {
            CompilerNoFlags       = 0x00,
            CompilerBuiltin       = 0x01,
            CompilerNoCheckDeps   = 0x02,
            CompilerRemoveNoExist = 0x04
        };
        uint flags, type;
    };
    void initCompiler(const Compiler &comp);

    //subclasses can use these to query information about how the generator was "run"
    QString buildArgs();
    QString specdir();

    virtual QStringList &findDependencies(const QString &file);
    virtual bool doDepends() const { return Option::mkfile::do_deps; }

    //for cross-platform dependent directories
    virtual void usePlatformDir();

    //for installs
    virtual QString defaultInstall(const QString &);

    //for prl
    void writePrlFile();
    bool processPrlFile(QString &);
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual void processPrlFiles();
    virtual void writePrlFile(QTextStream &);

    //make sure libraries are found
    virtual bool findLibraries();

    //for retrieving values and lists of values
    virtual QString var(const QString &var);
    QString varGlue(const QString &var, const QString &before, const QString &glue, const QString &after);
    QString varList(const QString &var);
    QString val(const QStringList &varList);
    QString valGlue(const QStringList &varList, const QString &before, const QString &glue, const QString &after);
    QString valList(const QStringList &varList);

    QString filePrefixRoot(const QString &, const QString &);

    //file fixification to unify all file names into a single pattern
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

    void setNoIO(bool o);
    bool noIO() const;

    inline bool exists(QString file) const { return fileInfo(file).exists(); }
    QFileInfo fileInfo(QString file) const;

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
