/****************************************************************************
** $Id: //depot/qt/main/src/%s#3 $
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
#ifndef __QMAKE_H__
#define __QMAKE_H__

#include <qstringlist.h>
#include <qstring.h>
#include <qmap.h>

class QMakeProject
{
    int scope_block, scope_flag;

    QString pfile, cfile;
    QMap<QString, QStringList> vars, base_vars, cache;
    bool read(const char *file, QMap<QString, QStringList> &place);
    bool parse(QString file, QString text, QMap<QString, QStringList> &place);
    bool doProjectTest(QString func, const QStringList &args, QMap<QString, QStringList> &place);
    void doProjectCheckReqs(const QStringList &deps);

public:
    QMakeProject();
    ~QMakeProject() { }

    bool read(QString project, QString pwd);
    QString projectFile() { return pfile == "-" ? QString("(stdin)") : pfile; }
    QString configFile() { return cfile; }

    bool isEmpty(const QString &v) { return !vars.contains(v) || vars[v].isEmpty(); }
    QStringList &values(const QString &v) { return vars[v]; }
    QString first(const QString &v) { return isEmpty(v) ? QString("") : vars[v].first(); }
    QMap<QString, QStringList> &variables() { return vars; }
    bool isActiveConfig(const QString &x);

};

#endif /* __QMAKE_H__ */
