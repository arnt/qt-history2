/****************************************************************************
**
** Definition of MakefileGenerator class.
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
    bool init_opath_already, init_already, moc_aware, no_io;
    QStringList createObjectList(const QString &var);
    QString build_args();
    void checkMultipleDefinition(const QString &, const QString &);
    QString createMocFileName(const QString &);
    QMap<QString, QString> fileFixed;
    QMap<QString, QMakeLocalFileName> depHeuristics;
    QMap<QString, QStringList> depends;

protected:
    void writeObj(QTextStream &, const QString &obj, const QString &src);
    void writeUicSrc(QTextStream &, const QString &ui);
    void writeMocObj(QTextStream &, const QString &obj, const QString &src);
    void writeMocSrc(QTextStream &, const QString &src);
    void writeLexSrc(QTextStream &, const QString &lex);
    void writeYaccSrc(QTextStream &, const QString &yac);
    void writeInstalls(QTextStream &t, const QString &installs);
    void writeImageObj(QTextStream &t, const QString &obj);
    void writeImageSrc(QTextStream &t, const QString &images);

    //interface to the source file info
    QMakeLocalFileName findFileForDep(const QMakeLocalFileName &);
    QMakeLocalFileName findFileForMoc(const QMakeLocalFileName &);

    QMakeProject *project;

    QString buildArgs();
    QString specdir();

    virtual QStringList &findDependencies(const QString &file);

    void setNoIO(bool o);
    bool noIO() const;

    void setMocAware(bool o);
    bool mocAware() const;

    virtual bool doDepends() const { return Option::mkfile::do_deps; }
    bool writeHeader(QTextStream &);
    virtual bool writeMakefile(QTextStream &);
    virtual bool writeMakeQmake(QTextStream &);
    void initOutPaths();
    virtual void init();

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
    virtual QString findDependency(const QString &);

    virtual QString var(const QString &var);
    QString varGlue(const QString &var, const QString &before, const QString &glue, const QString &after);
    QString varList(const QString &var);
    QString val(const QStringList &varList);
    QString valGlue(const QStringList &varList, const QString &before, const QString &glue, const QString &after);
    QString valList(const QStringList &varList);


    QString fileFixify(const QString& file, const QString &out_dir=QString::null,
		       const QString &in_dir=QString::null, bool force_fix=FALSE, bool canon=TRUE) const;
    QStringList fileFixify(const QStringList& files, const QString &out_dir=QString::null,
			   const QString &in_dir=QString::null, bool force_fix=FALSE, bool canon=TRUE) const;
public:
    MakefileGenerator(QMakeProject *p);
    virtual ~MakefileGenerator();

    static MakefileGenerator *create(QMakeProject *);
    virtual bool write();
    virtual bool openOutput(QFile &) const;
};

inline void MakefileGenerator::setMocAware(bool o)
{ moc_aware = o; }

inline bool MakefileGenerator::mocAware() const
{ return moc_aware; }

inline void MakefileGenerator::setNoIO(bool o)
{ no_io = o; }

inline bool MakefileGenerator::noIO() const
{ return no_io; }

inline QString MakefileGenerator::defaultInstall(const QString &)
{ return QString(""); }

inline bool MakefileGenerator::findLibraries()
{ return TRUE; }

inline QString MakefileGenerator::findDependency(const QString &)
{ return QString(""); }

inline MakefileGenerator::~MakefileGenerator()
{ }

QString mkdir_p_asstring(const QString &dir);

#endif /* __MAKEFILE_H__ */
