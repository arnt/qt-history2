/****************************************************************************
**
** Implementation of MySQL driver plugin
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
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

#include "../../qsqldriverinterface.h"
#include "qsql_mysql.h"
#include <qstringlist.h>

class QMySQLDriverInterface : public QSqlDriverInterface
{
public:
    QMySQLDriverInterface(){}

    QSqlDriver* create( const QString &name );
    QStringList featureList() const;
};

QSqlDriver* QMySQLDriverInterface::create( const QString &name )
{
    if ( name == "QMYSQL" )
	return new QMySQLDriver();
    return 0;
}

QStringList QMySQLDriverInterface::featureList() const
{
    QStringList l;
    l.append("QMYSQL");
    return l;
}

class QMySQLDriverPlugIn : public QPlugInInterface
{
public:
    QStringList interfaceList() const;
    QUnknownInterface* queryInterface( const QString& request );
};

QStringList QMySQLDriverPlugIn::interfaceList() const
{
    QStringList list;

    list << "QMySQLDriverInterface";

    return list;
}

QUnknownInterface* QMySQLDriverPlugIn::queryInterface( const QString& request )
{
    if ( request == "QMySQLDriverInterface" )
	return new QMySQLDriverInterface;
    return 0;
}

Q_EXPORT_INTERFACE(QMySQLDriverPlugIn)
