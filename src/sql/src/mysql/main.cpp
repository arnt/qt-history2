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

class QMYSQLDriverPlugin : public QSqlDriverInterface
{
public:
    QMYSQLDriverPlugin();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QSqlDriver* create( const QString &name );
    QStringList featureList() const;

private:
    unsigned long ref;
};

QMYSQLDriverPlugin::QMYSQLDriverPlugin()
: ref( 0 )
{
}

QUnknownInterface *QMYSQLDriverPlugin::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QFeatureListInterface )
	iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_QSqlDriverInterface )
	iface = (QSqlDriverInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long QMYSQLDriverPlugin::addRef()
{
    return ref++;
}

unsigned long QMYSQLDriverPlugin::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QSqlDriver* QMYSQLDriverPlugin::create( const QString &name )
{
    if ( name.upper() == "QMYSQL3" )
	return new QMYSQLDriver();
    return 0;
}

QStringList QMYSQLDriverPlugin::featureList() const
{
    QStringList l;
    l  << "QMYSQL3";
    return l;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( QMYSQLDriverPlugin )
}
