/****************************************************************************
** $Id: $
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

#include "project.h"
#include <qtextstream.h>

class MakefileGenerator
{
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
    QMap<QString, QString> mocablesToMOC, mocablesFromMOC;

protected:

    QMakeProject *project;
    QMap<QString, QStringList> depends;

    QString cleanFilePath(const QString &file) const;
    bool generateDependancies(QStringList &dirs, QString x);
    bool generateMocList(QString fn);

    inline QString findMocSource(const QString &moc_file) const 
	{ QString tmp = cleanFilePath(moc_file); return !mocablesFromMOC.contains(tmp) ? QString("") : mocablesFromMOC[tmp]; }
    inline QString findMocDestination(const QString &src_file) const 
	{ QString tmp = cleanFilePath(src_file); return !mocablesToMOC.contains(tmp) ? QString("") : mocablesToMOC[tmp]; }

    void setMocAware(bool o) { moc_aware = o; }
    bool mocAware() const { return moc_aware; }

    bool writeHeader(QTextStream &);
    virtual bool writeMakefile(QTextStream &);
    virtual bool writeMakeQmake(QTextStream &);
    virtual void init();

    //for installs
    virtual QString defaultInstall(const QString &) { return QString(""); }

    QString var(const QString &var);
    QString varGlue(const QString &var, const QString &before, const QString &glue, const QString &after);
    QString varList(const QString &var);

    bool fileFixify(QString &file) const;
    bool fileFixify(QStringList &files) const;
public:
    MakefileGenerator(QMakeProject *p);
    virtual ~MakefileGenerator() { }

    bool write();
};

#endif /* __MAKEFILE_H__ */
