/****************************************************************************
** $Id$
**
** Definition of ________ class.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __MAKEFILE_H__
#define __MAKEFILE_H__

#include "option.h"
#include "project.h"
#include <qtextstream.h>

class MakefileGenerator
{
    QString spec;
    bool init_already, moc_aware;
    QStringList createObjectList(const QString &var);
    QString build_args();
    void writeObj(QTextStream &, const QString &obj, const QString &src);
    void writeUicSrc(QTextStream &, const QString &ui);
    void writeMocObj(QTextStream &, const QString &obj);
    void writeMocSrc(QTextStream &, const QString &src);
    void writeLexSrc(QTextStream &, const QString &lex);
    void writeYaccSrc(QTextStream &, const QString &yac);
    void writeInstalls(QTextStream &t, const QString &installs);
    void writeImageObj(QTextStream &t, const QString &obj);
    void writeImageSrc(QTextStream &t, const QString &images);
    QMap<QString, QString> mocablesToMOC, mocablesFromMOC;

protected:

    QMakeProject *project;
    QMap<QString, QStringList> depends;

    class MakefileDependDir {
    public:
	MakefileDependDir(QString r, QString l) : real_dir(r), local_dir(l) { }
	QString real_dir, local_dir;
    };
    bool generateDependencies(QPtrList<MakefileDependDir> &dirs, QString x, bool recurse);

    QString buildArgs();

    QString specdir();
    QString cleanFilePath(const QString &file) const;
    bool generateMocList(QString fn);

    QString findMocSource(const QString &moc_file) const;
    QString findMocDestination(const QString &src_file) const;
    QStringList &findDependencies(const QString &file);

    void setMocAware(bool o);
    bool mocAware() const;
    void logicWarn(const QString &, const QString &);

    virtual bool doDepends() const { return Option::mkfile::do_deps; }
    bool writeHeader(QTextStream &);
    virtual bool writeMakefile(QTextStream &);
    virtual bool writeMakeQmake(QTextStream &);
    virtual void init();

    //for installs
    virtual QString defaultInstall(const QString &);

    //for prl
    bool processPrlFile(QString &);
    virtual void processPrlFiles();

    QString var(const QString &var);
    QString varGlue(const QString &var, const QString &before, const QString &glue, const QString &after);
    QString varList(const QString &var);

    bool fileFixify(QString &file, QString dir=QString::null) const;
    bool fileFixify(QStringList &files, QString dir=QString::null) const;
public:
    MakefileGenerator(QMakeProject *p);
    virtual ~MakefileGenerator();

    bool write();
};

inline QString MakefileGenerator::findMocSource(const QString &moc_file) const 
{
    QString tmp = cleanFilePath(moc_file);
    if (mocablesFromMOC.contains(tmp))
	return mocablesFromMOC[tmp];
    else
	return QString("");
}

inline QString MakefileGenerator::findMocDestination(const QString &src_file) const 
{
    QString tmp = cleanFilePath(src_file);
    if (mocablesToMOC.contains(tmp))
	return mocablesToMOC[tmp];
    else
	return QString("");
}

inline void MakefileGenerator::setMocAware(bool o)
{ moc_aware = o; }

inline bool MakefileGenerator::mocAware() const
{ return moc_aware; }

inline QString MakefileGenerator::defaultInstall(const QString &)
{ return QString(""); }

inline MakefileGenerator::~MakefileGenerator()
{ }


#endif /* __MAKEFILE_H__ */
